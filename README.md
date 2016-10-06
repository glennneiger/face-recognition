# Clemson FCT Facial Recognition

This repository contains the code for a face recognition system that combines three popular algorithms: PCA, LDA, and ICA.

## Getting Started

New team members should look at CONTRIBUTING.md to learn about our work-flow, especially those who are unfamiliar with Github.

## Testing

Usage for the face recognition system:

    Usage: ./face-rec [options]
    Options:
      --train DIRECTORY  create a database from a training set
      --rec DIRECTORY    test a set of images against a database
      --pca              run PCA
      --lda              run LDA
      --ica              run ICA
      --all              run all algorithms (PCA, LDA, ICA)

To run an automated test (Monte Carlo cross-validation) with the ORL face database:

    # test with 3 random samples removed, 10 iterations
    ./scripts/cross-validate.sh -p orl_faces -e pgm -t 3 -i 10 [--pca --lda --ica --all]

To test MATLAB code with ORL database:

    # test with 3 random samples removed, 10 iterations
    ./scripts/cross-validate-matlab.sh -p orl_faces -e pgm -t 3 -n 10 [--pca --lda --ica]

## The Image Library

This software currently supports a subset of the [Netpbm](https://en.wikipedia.org/wiki/Netpbm_format) format, particularly with PGM and PPM images.

Images should __not__ be stored in this repository! Instead, images should be downloaded separately. Face databases are widely available on the Internet, such as [here](http://web.mit.edu/emeyers/www/face_databases.html) and [here](http://face-rec.org/databases/). I am currently using the [ORL database](http://www.cl.cam.ac.uk/research/dtg/attarchive/facedatabase.html):

    wget http://www.cl.cam.ac.uk/Research/DTG/attarchive/pub/data/att_faces.tar.Z
    tar -xvf att_faces.tar.Z
    rm orl_faces/README

To convert JPEG images to PGM with ImageMagick:

    ./scripts/convert-images.sh [src-folder] [dst-folder] jpeg pgm

## Results

Not quite ready

## The Algorithms

Here is the working flow graph for the combined algorithm:

    m = number of dimensions per image
    n = number of images

    train: X -> (a, W', P)
        X = [X_1 ... X_n] (image matrix) (m-by-n)
        a = sum(X_i, 1:i:n) / n (mean face) (m-by-1)
        X = [(X_1 - a) ... (X_n - a)] (mean-subtracted image matrix) (m-by-n)
        W_pca' = PCA(X) (PCA projection matrix) (n-by-m)
        P_pca = W_pca' * X (PCA projected image matrix) (n-by-n)
        W_lda' = LDA(W_pca, P_pca) (LDA projection matrix) (n-by-m)
        P_lda = W_lda' * X (LDA projected image matrix) (n-by-n)
        W_ica' = ICA(W_pca, P_pca) (ICA projection matrix) (n-by-m)
        P_ica = W_ica' * X (ICA projected image matrix) (n-by-n)

    recognize: X_test -> P_match
        a = mean face (m-by-1)
        (W_pca, W_lda, W_ica) = projection matrices (m-by-n)
        (P_pca, P_lda, P_ica) = projected image matrices (n-by-n)
        X_test = test image (m-by-1)
        P_test_pca = W_pca' * (X_test - a) (n-by-1)
        P_test_lda = W_lda' * (X_test - a) (n-by-1)
        P_test_ica = W_ica' * (X_test - a) (n-by-1)
        P_match_pca = nearest neighbor of P_test_pca (n-by-1)
        P_match_lda = nearest neighbor of P_test_lda (n-by-1)
        P_match_ica = nearest neighbor of P_test_ica (n-by-1)

    PCA: X -> W_pca'
        X = [X_1 ... X_n] (image matrix) (m-by-n)
        L = X' * X (surrogate matrix) (n-by-n)
        L_ev = eigenvectors of L (n-by-n)
        W_pca = X * L_ev (eigenfaces) (m-by-n)

    LDA: (X, W_pca, c, n_opt1) -> W_lda'
        c = number of classes
        n_opt1 = number of columns to take from W_pca
        W_pca2 = W_pca(1 : n_opt1) (m by n_opt1)
        P_pca = W_pca2' * X (n_opt1 by n)
        n_i = size of class i
        U_i = sum(P_pca_j, j in class i) / n_i (n_opt1 by 1)
        u = sum(U_i, 1:i:c) / c (n_opt1 by 1)
        S_b = sum((U_i - u) * (U_i - u)', 1:i:c) (n_opt1 by n_opt1)
        S_w = sum(sum((P_pca_j - U_i) * (P_pca_j - U_i)', j in class i), 1:i:c) (n_opt1 by n_opt1)
        W_fld = eigenvectors of (S_b, S_w) (n_opt1 by n_opt1)
        W_lda' = W_fld' * W_pca2' (n by m)

    ICA: (W_pca, P_pca) -> W_ica'
        X = P_pca (n-by-n)
        W_z = 2 * Cov(X)^(-1/2) (n-by-n)
        X_sph = W_z * X (n-by-n)
        W = (train with sep96) (n-by-n)
        W_I = W * W_z (n-by-n)
        W_ica' = W_I * W_pca' (n-by-m)

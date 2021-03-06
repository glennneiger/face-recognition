/**
 * @file main.cpp
 *
 * User interface to the face recognition system.
 */
#include <cstdlib>
#include <exception>
#include <getopt.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <mlearn.h>
#include <unistd.h>

using namespace ML;

enum class DataType {
	None,
	Genome,
	Image
};

enum class FeatureType {
	Identity,
	PCA,
	LDA,
	ICA
};

enum class ClassifierType {
	None,
	KNN,
	Bayes
};

typedef enum {
	OPTION_GPU,
	OPTION_LOGLEVEL,
	OPTION_TRAIN,
	OPTION_TEST,
	OPTION_STREAM,
	OPTION_DATA,
	OPTION_PCA,
	OPTION_LDA,
	OPTION_ICA,
	OPTION_KNN,
	OPTION_BAYES,
	OPTION_PCA_N1,
	OPTION_LDA_N1,
	OPTION_LDA_N2,
	OPTION_ICA_N1,
	OPTION_ICA_N2,
	OPTION_ICA_NONL,
	OPTION_ICA_MAX_ITER,
	OPTION_ICA_EPS,
	OPTION_KNN_K,
	OPTION_KNN_DIST,
	OPTION_UNKNOWN = '?'
} option_t;

typedef struct {
	bool train;
	bool test;
	bool stream;
	const char *path_train;
	const char *path_test;
	const char *path_stream;
	const char *path_model;
	DataType data_type;
	FeatureType feature_type;
	ClassifierType classifier_type;
	int pca_n1;
	int lda_n1;
	int lda_n2;
	int ica_n1;
	int ica_n2;
	ICANonl ica_nonl;
	int ica_max_iter;
	float ica_eps;
	int knn_k;
	dist_func_t knn_dist;
} optarg_t;

const std::map<std::string, DataType> data_types = {
	{ "genome", DataType::Genome },
	{ "image", DataType::Image }
};

const std::map<std::string, dist_func_t> dist_funcs = {
	{ "COS", m_dist_COS },
	{ "L1", m_dist_L1 },
	{ "L2", m_dist_L2 }
};

const std::map<std::string, ICANonl> nonl_funcs = {
	{ "pow3", ICANonl::pow3 },
	{ "tanh", ICANonl::tanh },
	{ "gauss", ICANonl::gauss }
};

/**
 * Print command-line usage and help text.
 */
void print_usage()
{
	std::cerr <<
		"Usage: ./face-rec [options]\n"
		"\n"
		"Options:\n"
		"  --gpu              enable GPU acceleration\n"
		"  --loglevel LEVEL   set the log level ([1]=info, 2=verbose, 3=debug)\n"
		"  --train DIR        train a model with a training set\n"
		"  --test DIR         perform recognition on a test set\n"
		"  --stream DIR       perform recognition on an input stream\n"
		"  --data             data type (genome, [image])\n"
		"  --pca              use PCA for feature extraction\n"
		"  --lda              use LDA for feature extraction\n"
		"  --ica              use ICA for feature extraction\n"
		"  --knn              use the kNN classifier (default)\n"
		"  --bayes            use the Bayes classifier\n"
		"\n"
		"Hyperparameters:\n"
		"PCA:\n"
		"  --pca_n1 N         number of principal components to compute\n"
		"\n"
		"LDA:\n"
		"  --lda_n1 N         number of principal components to compute\n"
		"  --lda_n2 N         number of Fisherfaces to compute\n"
		"\n"
		"ICA:\n"
		"  --ica_n1 N         number of principal components to compute\n"
		"  --ica_n2 N         number of independent components to estimate\n"
		"  --ica_nonl [nonl]  nonlinearity function to use ([pow3], tanh, gauss)\n"
		"  --ica_max_iter N   maximum iterations\n"
		"  --ica_eps X        convergence threshold for w\n"
		"\n"
		"kNN:\n"
		"  --knn_k N          number of nearest neighbors to use\n"
		"  --knn_dist [dist]  distance function to use (L1, [L2], COS)\n";
}

/**
 * Parse command-line arguments.
 *
 * @param argc
 * @param argv
 */
optarg_t parse_args(int argc, char **argv)
{
	optarg_t args = {
		false,
		false,
		false,
		nullptr,
		nullptr,
		nullptr,
		"./model.dat",
		DataType::Image,
		FeatureType::Identity,
		ClassifierType::KNN,
		-1,
		-1, -1,
		-1, -1, ICANonl::pow3, 1000, 0.0001f,
		1, m_dist_L2
	};

	struct option long_options[] = {
		{ "gpu", no_argument, 0, OPTION_GPU },
		{ "loglevel", required_argument, 0, OPTION_LOGLEVEL },
		{ "train", required_argument, 0, OPTION_TRAIN },
		{ "test", required_argument, 0, OPTION_TEST },
		{ "stream", required_argument, 0, OPTION_STREAM },
		{ "data", required_argument, 0, OPTION_DATA },
		{ "pca", no_argument, 0, OPTION_PCA },
		{ "lda", no_argument, 0, OPTION_LDA },
		{ "ica", no_argument, 0, OPTION_ICA },
		{ "knn", no_argument, 0, OPTION_KNN },
		{ "bayes", no_argument, 0, OPTION_BAYES },
		{ "pca_n1", required_argument, 0, OPTION_PCA_N1 },
		{ "lda_n1", required_argument, 0, OPTION_LDA_N1 },
		{ "lda_n2", required_argument, 0, OPTION_LDA_N2 },
		{ "ica_n1", required_argument, 0, OPTION_ICA_N1 },
		{ "ica_n2", required_argument, 0, OPTION_ICA_N2 },
		{ "ica_nonl", required_argument, 0, OPTION_ICA_NONL },
		{ "ica_max_iter", required_argument, 0, OPTION_ICA_MAX_ITER },
		{ "ica_eps", required_argument, 0, OPTION_ICA_EPS },
		{ "knn_k", required_argument, 0, OPTION_KNN_K },
		{ "knn_dist", required_argument, 0, OPTION_KNN_DIST },
		{ 0, 0, 0, 0 }
	};

	int opt;
	while ( (opt = getopt_long_only(argc, argv, "", long_options, nullptr)) != -1 ) {
		switch ( opt ) {
		case OPTION_GPU:
			GPU = true;
			break;
		case OPTION_LOGLEVEL:
			LOGLEVEL = (logger_level_t) atoi(optarg);
			break;
		case OPTION_TRAIN:
			args.train = true;
			args.path_train = optarg;
			break;
		case OPTION_TEST:
			args.test = true;
			args.path_test = optarg;
			break;
		case OPTION_STREAM:
			args.stream = true;
			args.path_stream = optarg;
			break;
		case OPTION_DATA:
			try {
				args.data_type = data_types.at(optarg);
			}
			catch ( std::exception& e ) {
				args.data_type = DataType::None;
			}
			break;
		case OPTION_PCA:
			args.feature_type = FeatureType::PCA;
			break;
		case OPTION_LDA:
			args.feature_type = FeatureType::LDA;
			break;
		case OPTION_ICA:
			args.feature_type = FeatureType::ICA;
			break;
		case OPTION_KNN:
			args.classifier_type = ClassifierType::KNN;
			break;
		case OPTION_BAYES:
			args.classifier_type = ClassifierType::Bayes;
			break;
		case OPTION_PCA_N1:
			args.pca_n1 = atoi(optarg);
			break;
		case OPTION_LDA_N1:
			args.lda_n1 = atoi(optarg);
			break;
		case OPTION_LDA_N2:
			args.lda_n2 = atoi(optarg);
			break;
		case OPTION_ICA_N1:
			args.ica_n1 = atoi(optarg);
			break;
		case OPTION_ICA_N2:
			args.ica_n2 = atoi(optarg);
			break;
		case OPTION_ICA_NONL:
			try {
				args.ica_nonl = nonl_funcs.at(optarg);
			}
			catch ( std::exception& e ) {
				args.ica_nonl = ICANonl::none;
			}
			break;
		case OPTION_ICA_MAX_ITER:
			args.ica_max_iter = atoi(optarg);
			break;
		case OPTION_ICA_EPS:
			args.ica_eps = atof(optarg);
			break;
		case OPTION_KNN_K:
			args.knn_k = atoi(optarg);
			break;
		case OPTION_KNN_DIST:
			try {
				args.knn_dist = dist_funcs.at(optarg);
			}
			catch ( std::exception& e ) {
				args.knn_dist = nullptr;
			}
			break;
		case OPTION_UNKNOWN:
			print_usage();
			exit(1);
		}
	}

	return args;
}

/**
 * Validate command-line arguments.
 *
 * @param args
 */
void validate_args(const optarg_t& args)
{
	std::vector<std::pair<bool, std::string>> validators = {
		{ args.train || args.test || args.stream, "--train / --test / --stream is required" },
		{ args.data_type != DataType::None, "--data must be genome | image" },
		{ args.knn_dist != nullptr, "--knn_dist must be L1 | L2 | COS" },
		{ args.ica_nonl != ICANonl::none, "--ica_nonl must be pow3 | tanh | gauss" }
	};
	bool valid = true;

	for ( auto v : validators ) {
		if ( !v.first ) {
			std::cerr << "error: " << v.second << "\n";
			valid = false;
		}
	}

	if ( !valid ) {
		print_usage();
		exit(1);
	}
}

int main(int argc, char **argv)
{
	// parse command-line arguments
	optarg_t args = parse_args(argc, argv);

	// validate arguments
	validate_args(args);

	// initialize random number engine
	RNG_seed();

	// initialize GPU if enabled
	gpu_init();

	// initialize data type
	std::unique_ptr<DataIterator> data_iter;

	if ( args.data_type == DataType::Genome ) {
		data_iter.reset(new Genome());
	}
	else if ( args.data_type == DataType::Image ) {
		data_iter.reset(new Image());
	}

	// initialize feature layer
	std::unique_ptr<FeatureLayer> feature;

	if ( args.feature_type == FeatureType::Identity ) {
		feature.reset(new IdentityLayer());
	}
	else if ( args.feature_type == FeatureType::PCA ) {
		feature.reset(new PCALayer(args.pca_n1));
	}
	else if ( args.feature_type == FeatureType::LDA ) {
		feature.reset(new LDALayer(args.lda_n1, args.lda_n2));
	}
	else if ( args.feature_type == FeatureType::ICA ) {
		feature.reset(new ICALayer(
			args.ica_n1,
			args.ica_n2,
			args.ica_nonl,
			args.ica_max_iter,
			args.ica_eps
		));
	}

	// initialize classifier layer
	std::unique_ptr<ClassifierLayer> classifier;

	if ( args.classifier_type == ClassifierType::KNN ) {
		classifier.reset(new KNNLayer(args.knn_k, args.knn_dist));
	}
	else if ( args.classifier_type == ClassifierType::Bayes ) {
		classifier.reset(new BayesLayer());
	}

	// initialize model
	ClassificationModel model(feature.get(), classifier.get());

	// run the face recognition system
	if ( args.train ) {
		Dataset train_set(data_iter.get(), args.path_train);

		model.train(train_set);
	}
	else {
		model.load(args.path_model);
	}

	if ( args.test ) {
		Dataset test_set(data_iter.get(), args.path_test);

		std::vector<DataLabel> Y_pred = model.predict(test_set);

		model.validate(test_set, Y_pred);
		model.print_results(test_set, Y_pred);
	}
	else if ( args.stream ) {
		const char END = '0';
		const char READ = '1';

		while ( true ) {
			char c = std::cin.get();

			if ( c == END ) {
				break;
			}
			else if ( c == READ ) {
				Dataset test_set(data_iter.get(), args.path_stream, false);

				std::vector<DataLabel> Y_pred = model.predict(test_set);

				// print results
				for ( size_t i = 0; i < test_set.entries().size(); i++ ) {
					const DataLabel& y_pred = Y_pred[i];
					const DataEntry& entry = test_set.entries()[i];

					std::cout << std::left << std::setw(12) << entry.name << "  " << y_pred << "\n";
				}
			}
		}
	}
	else {
		model.save(args.path_model);
	}

	timer_print();

	model.print_stats();

	gpu_finalize();

	return 0;
}

#pragma once
#include "evaluation.h"
#include "types.h"
#include <vector>
#include <functional>

namespace Tuner {

// Parameter definition
struct TuneParameter {
    std::string name;
    int* value;         // Pointer to engine parameter
    int min, max;       // Valid range
    int step;           // Adjustment granularity
};

// Optimization algorithm options
enum class TuneAlgorithm {
    SPSA,               // Simultaneous Perturbation Stochastic Approximation
    CLOP,               // Confidence-based Local Optimization
    GA                  // Genetic Algorithm
};

// Tuning configuration
struct TuneConfig {
    TuneAlgorithm algorithm = TuneAlgorithm::SPSA;
    int iterations = 5000;
    double aggression = 1.0; // Risk factor
    std::string save_file = "tuning.json";
};

// Main tuning interface
class ParameterTuner {
public:
    void add_parameter(TuneParameter param);
    void configure(const TuneConfig& config);
    
    void tune(const std::vector<Game>& games);
    void save_results() const;
    void load_results();

private:
    std::vector<TuneParameter> parameters;
    TuneConfig config;
    
    // Algorithm implementations
    void run_spsa(const std::vector<Game>& games);
    void run_clop(const std::vector<Game>& games);
    void run_ga(const std::vector<Game>& games);
    
    // Evaluation functions
    double evaluate_changes(const std::vector<int>& params, 
                          const std::vector<Game>& games);
    double game_result_score(const Game& game, const Eval::EvalTerms& terms);
};

// Global tuner instance
extern ParameterTuner Tuner;
}
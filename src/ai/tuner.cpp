#include "tuner.h"
#include "evaluation.h"
#include "search.h"
#include <random>
#include <cmath>
#include <fstream>
#include <nlohmann/json.hpp>

namespace Tuner {
ParameterTuner Tuner;

// Thread-safe random number generation
namespace {
    std::mutex rng_mutex;
    std::mt19937_64 rng{std::random_device{}()};
    
    double random_double(double min, double max) {
        std::uniform_real_distribution<double> dist(min, max);
        std::lock_guard<std::mutex> lock(rng_mutex);
        return dist(rng);
    }
}

void ParameterTuner::add_parameter(TuneParameter param) {
    parameters.push_back(param);
}

void ParameterTuner::configure(const TuneConfig& cfg) {
    config = cfg;
}

void ParameterTuner::tune(const std::vector<Game>& games) {
    switch (config.algorithm) {
        case TuneAlgorithm::SPSA: run_spsa(games); break;
        case TuneAlgorithm::CLOP: run_clop(games); break;
        case TuneAlgorithm::GA: run_ga(games); break;
    }
}

// SPSA Implementation (most effective for chess tuning)
void ParameterTuner::run_spsa(const std::vector<Game>& games) {
    const double a = 0.1 * config.aggression;
    const double c = 0.1;
    const double A = 0.1 * config.iterations;
    const double alpha = 0.602;
    const double gamma = 0.101;
    
    std::vector<double> current(parameters.size());
    for (size_t i = 0; i < parameters.size(); ++i) {
        current[i] = *parameters[i].value;
    }

    for (int k = 1; k <= config.iterations; ++k) {
        // Generate random perturbation
        std::vector<double> delta(parameters.size());
        for (size_t i = 0; i < parameters.size(); ++i) {
            delta[i] = (random_double(0, 1) > 0.5) ? 1 : -1;
        }
        
        // Evaluate both perturbations
        double ak = a / std::pow(A + k, alpha);
        double ck = c / std::pow(k, gamma);
        
        std::vector<int> params_plus, params_minus;
        for (size_t i = 0; i < parameters.size(); ++i) {
            params_plus.push_back(static_cast<int>(current[i] + ck * delta[i]));
            params_minus.push_back(static_cast<int>(current[i] - ck * delta[i]));
        }
        
        double score_plus = evaluate_changes(params_plus, games);
        double score_minus = evaluate_changes(params_minus, games);
        
        // Gradient approximation
        double gradient_scale = (score_plus - score_minus) / (2.0 * ck);
        for (size_t i = 0; i < parameters.size(); ++i) {
            current[i] = std::clamp(
                current[i] + ak * gradient_scale * delta[i],
                static_cast<double>(parameters[i].min),
                static_cast<double>(parameters[i].max)
            );
            
            // Update actual engine parameter
            *parameters[i].value = static_cast<int>(std::round(current[i]));
        }
        
        if (k % 100 == 0) {
            save_results();
        }
    }
}

double ParameterTuner::evaluate_changes(const std::vector<int>& params, 
                                      const std::vector<Game>& games) {
    // Apply temporary parameters
    for (size_t i = 0; i < parameters.size(); ++i) {
        *parameters[i].value = params[i];
    }
    
    // Evaluate on all games
    double total = 0.0;
    for (const auto& game : games) {
        total += game_result_score(game, Eval::current_terms());
    }
    
    return total / games.size();
}

double ParameterTuner::game_result_score(const Game& game, 
                                       const Eval::EvalTerms& terms) {
    // Use logistic function to convert eval to expected score
    const double eval = Eval::evaluate(game.final_position, terms);
    const double expected = 1.0 / (1.0 + std::exp(-eval / 200.0));
    
    // Compare to actual result (0=loss, 0.5=draw, 1=win)
    const double actual = (game.result == GameResult::DRAW) ? 0.5 : 
                        (game.result == game.side_to_move ? 1.0 : 0.0);
    
    // Quadratic loss function
    return 1.0 - std::pow(actual - expected, 2);
}

void ParameterTuner::save_results() const {
    nlohmann::json j;
    for (const auto& param : parameters) {
        j[param.name] = *param.value;
    }
    
    std::ofstream file(config.save_file);
    file << j.dump(4);
}

void ParameterTuner::load_results() {
    std::ifstream file(config.save_file);
    if (!file) return;
    
    nlohmann::json j;
    file >> j;
    
    for (auto& param : parameters) {
        if (j.contains(param.name)) {
            *param.value = j[param.name].get<int>();
        }
    }
}
} // namespace Tuner
#include <iostream>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>

// Define a WindTurbine class to encapsulate turbine data and calculations
class WindTurbine {
public:
    WindTurbine() {
        // Define turbine power curve coefficients
        p_coefficients = {6.4563e-5, -8.3, 225.2, -1481.6, 3056.8};
        air_density = 1.225;  // Air density at sea level
    }

    // Calculate turbine power output for a given wind speed
    double calculatePower(double wind_speed) {
        if (wind_speed < 4.0)
            return 0.0;
        else if (wind_speed >= 4.0 && wind_speed < 15.0)
            return p_coefficients[0] * pow(wind_speed, 4) +
                   p_coefficients[1] * pow(wind_speed, 3) +
                   p_coefficients[2] * pow(wind_speed, 2) +
                   p_coefficients[3] * wind_speed +
                   p_coefficients[4];
        else
            return 3600.0;
    }

private:
    std::vector<double> p_coefficients;
    double air_density;
};

int main() {
    // Create a WindTurbine object
    WindTurbine turbine;

    // Seed the random number generator
    std::random_device rd;
    std::mt19937 gen(rd());

    // Define Weibull parameters
    double A = 6.677;  // Replace with your Weibull parameter
    std::normal_distribution<double> B_distribution(2.5562, 0.0076);

    // Define variables to store results
    const int num_simulations = 100;
    std::vector<double> power_conventional(num_simulations);
    std::vector<double> power_results(num_simulations);

    for (int j = 0; j < num_simulations; ++j) {
        // Generate B value using normal distribution
        double B = B_distribution(gen);

        // Generate hourly wind values using Weibull distribution
        std::vector<double> v_10(8760);
        for (int i = 0; i < 8760; ++i) {
            v_10[i] = std::tgamma(1.0 + 1.0 / A) /
                      pow(std::log(2.0), 1.0 / A) *
                      pow(-std::log(1.0 - (double)rand() / RAND_MAX), 1.0 / A);
        }

        // Extrapolate wind velocity to 90m above sea level
        std::vector<double> v(8760);
        for (int i = 0; i < 8760; ++i) {
            v[i] = v_10[i] * (std::log(90.0 / 0.0002) / std::log(10.0 / 0.0002));
        }

        // Calculate power and standard deviation
        std::vector<double> p(8760);
        std::vector<double> sig(8760);
        for (int i = 0; i < 8760; ++i) {
            p[i] = turbine.calculatePower(v[i]);
            sig[i] = ((v[i] >= 4.0 && v[i] < 15.0) ?
                (-0.35 * (v[i] - 15.0) / 11.0 + 0.1) * p[i] / 2.4495 : 0.1 * p[i] / 2.4495);
        }

        // Calculate power considering standard deviation
        std::vector<double> N(8760);
        for (int i = 0; i < 8760; ++i) {
            N[i] = std::normal_distribution<double>(p[i], sig[i])(gen);
        }

        // Calculate final power output
        double total_power = std::accumulate(N.begin(), N.end(), 0.0);
        power_conventional[j] = total_power;
    }

    // Calculate average annual energy production
    double AEP_con = std::accumulate(power_conventional.begin(), power_conventional.end(), 0.0) / num_simulations;

    std::cout << "Average Annual Energy Production (Conventional): " << AEP_con << " kWh" << std::endl;

    return 0;
}

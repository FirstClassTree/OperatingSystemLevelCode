import numpy as np
import matplotlib.pyplot as plt
import math

# Your code goes here

# Example usage of numpy


def generate_bernoulli(p, size=1):
    return np.random.binomial(1, p, size)

def main():
    bernoulli_data = generate_bernoulli(0.5, (200000,20))
    row_means = np.mean(bernoulli_data,axis=1)
    epsilon_values = np.linspace(0,1,50)
    print(epsilon_values)

    probabilites = []
    hoeffding_bounds = []
    for epsilon in epsilon_values:
        probabilites.append(np.mean(np.abs(row_means-0.5)>epsilon))
        hoeffding_bounds.append(2 * math.exp(-2 * 20 * epsilon**2))

    plt.plot(epsilon_values,probabilites, label='Empirical')
    plt.plot(epsilon_values, hoeffding_bounds, label='Hoeffding Bound')
    plt.xlabel("Epsilon")
    plt.ylabel('Probability')
    plt.title('Probability that |row mean - 0.5| > epsilon')
    plt.show()



if __name__ == "__main__":
    main()
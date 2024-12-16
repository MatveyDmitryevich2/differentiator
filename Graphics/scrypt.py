import matplotlib.pyplot as plt
import argparse

def plot_graph(data_file, output_pdf):
    x, y = [], []

    with open(data_file, 'r') as file:
        for line in file:
            values = line.split()
            x.append(float(values[0]))
            y.append(float(values[1]))

    plt.plot(x, y, marker='o', linestyle='-', color='g', label='Данные')
    plt.xlabel('X')
    plt.ylabel('Y')
    plt.legend()
    plt.savefig(output_pdf, format='pdf')

if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument('data_file', type=str)
    parser.add_argument('output_pdf', type=str)
    
    args = parser.parse_args()

    plot_graph(args.data_file, args.output_pdf)
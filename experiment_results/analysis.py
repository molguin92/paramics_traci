# python 3
import pandas
import numpy

def analysis():
    per00 = pandas.read_csv('./PER0.0_clean.csv', sep=';', index_col="Module", thousands=',')
    per10 = pandas.read_csv('./PER1.0_clean.csv', sep=';', index_col="Module", thousands=',')
    per00.sort_index(inplace=True)
    per10.sort_index(inplace=True)

    reached_dest_per00 = per00[per00['stopTime'] < 27900]
    reached_dest_per10 = per10[per10['stopTime'] < 27900]

    print('Modules that reached their destination before 15 minutes of simulation:')
    print('PER 0.0:', reached_dest_per00.shape[0])
    print('PER 1.0:', reached_dest_per10.shape[0])



if __name__ == '__main__':
    analysis()

# python 3
import os

import pandas
import re

import numpy
from threading import Thread
import matplotlib
from matplotlib import pyplot

matplotlib.style.use('ggplot')
pyplot.interactive(False)

def get_speed_stats(speed_data_path):
    df = pandas.read_csv(speed_data_path, sep=',', thousands=',')
    try:
        node_nr = re.search('RSUExampleScenario.(.+?).veinsmobility.speed', df.columns[1]).group(1)
    except AttributeError:
        node_nr = '??'  # apply your error handling
    df.columns = ['time', 'speed']
    mean = df['speed'].mean()
    std = df['speed'].std()

    return (node_nr, mean, std)

def store_speed_data_csv():

    per10_speedpath = os.path.abspath('.\PER1.0_100Demand\speed_data')
    per00_speedpath = os.path.abspath('.\PER0.0_100Demand_5secs\speed_data')

    per10_df = pandas.DataFrame(columns=['node', 'mean_speed', 'std'])
    per00_df = pandas.DataFrame(columns=['node', 'mean_speed', 'std'])

    for file in os.listdir(per10_speedpath):
        stats = get_speed_stats(os.path.join(per10_speedpath, file))
        per10_df.loc[len(per10_df)] = stats

    for file in os.listdir(per00_speedpath):
        stats = get_speed_stats(os.path.join(per00_speedpath, file))
        per00_df.loc[len(per00_df)] = stats

    per00_df.set_index(['node'], inplace=True)
    per10_df.set_index(['node'], inplace=True)

    per00_df.to_csv('speed_stats_per00.csv', encoding='utf-8')
    per10_df.to_csv('speed_stats_per10.csv', encoding='utf-8')

def analysis_speed():
    per00_df = pandas.read_csv('speed_stats_per00.csv')
    per10_df = pandas.read_csv('speed_stats_per10.csv')

    per00_df.set_index(['node'], inplace=True)
    per10_df.set_index(['node'], inplace=True)

    bins = numpy.linspace(0, per00_df['mean_speed'].max(), 50)

    pyplot.hist(per00_df['mean_speed'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_df['mean_speed'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de velocidades promedio')
    pyplot.show()

    #fig, axes = pyplot.subplots(nrows=1, ncols=2)
    #per00_df['mean_speed'].plot.hist(ax=axes[0], color='DarkBlue', bins=10)
    #per10_df['mean_speed'].plot.hist(ax=axes[1], color='DarkRed', bins=10)
    #pyplot.show()

def analysis_distance():
    per00_df = pandas.read_csv('./PER0.0_100Demand_5secs_clean.csv', sep=';')
    per10_df = pandas.read_csv('./PER1.0_100Demand_clean.csv', sep=';')

    per00_df.set_index(['Module'], inplace=True)
    per10_df.set_index(['Module'], inplace=True)

    bins = numpy.linspace(0, per00_df['totalDistance'].max(), 50)

    pyplot.hist(per00_df['totalDistance'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_df['totalDistance'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de distancias totales')
    pyplot.show()


def analysis_time():
    per00_df = pandas.read_csv('./PER0.0_100Demand_5secs_clean.csv', sep=';')
    per10_df = pandas.read_csv('./PER1.0_100Demand_clean.csv', sep=';')

    per00_df.set_index(['Module'], inplace=True)
    per10_df.set_index(['Module'], inplace=True)

    bins = numpy.linspace(0, 1000, 50)

    pyplot.hist(per00_df['totalTime'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_df['totalTime'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de tiempos totales')
    pyplot.show()




if __name__ == '__main__':
    analysis_speed()
    analysis_distance()
    analysis_time()

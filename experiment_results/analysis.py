# python 3
import os

import pandas
import re

import numpy
import matplotlib
from matplotlib import pyplot

matplotlib.style.use('ggplot')
pyplot.interactive(False)


def get_speed_stats(speed_data_path):
    df = pandas.read_csv(speed_data_path, sep=',', thousands=',')
    try:
        node_nr = re.search('ProvidenciaExampleScenario.(.+?).veinsmobility.speed', df.columns[1]).group(1)
    except AttributeError:
        node_nr = '??'  # apply your error handling
    df.columns = ['time', 'speed']
    mean = df['speed'].mean()
    std = df['speed'].std()

    return (node_nr, mean, std)


def build_csv():
    # mobility data
    mobility_columns = ['module', 'max_speed', 'min_speed', 'start_time', 'stop_time',
                        'total_co2', 'total_dist', 'total_time']
    per_00_df_mobility = pandas.read_csv('per0.0_stats_veinsmobility.csv')
    per_00_df_mobility.columns = mobility_columns

    per_10_df_mobility = pandas.read_csv('per1.0_stats_veinsmobility.csv')
    per_10_df_mobility.columns = mobility_columns

    mobility_search_re = 'ProvidenciaExampleScenario.(.+?).veinsmobility'
    per_00_df_mobility['module'] = per_00_df_mobility['module'].map(lambda x: re.search(mobility_search_re, x).group(1))
    per_10_df_mobility['module'] = per_10_df_mobility['module'].map(lambda x: re.search(mobility_search_re, x).group(1))

    per_00_df_mobility.set_index(['module'], inplace=True)
    per_10_df_mobility.set_index(['module'], inplace=True)

    # appl data (sent warnings, arrived at dest)
    appl_columns = ['module', 'arrived', 'rcvd_warnings', 'sent_warnings']
    per_00_df_appl = pandas.read_csv('per0.0_stats_appl.csv')
    per_00_df_appl.columns = appl_columns

    per_10_df_appl = pandas.read_csv('per1.0_stats_appl.csv')
    per_10_df_appl.columns = appl_columns

    appl_search_re = 'ProvidenciaExampleScenario.(.+?).appl'
    per_00_df_appl['module'] = per_00_df_appl['module'].map(lambda x: re.search(appl_search_re, x).group(1))
    per_10_df_appl['module'] = per_10_df_appl['module'].map(lambda x: re.search(appl_search_re, x).group(1))

    per_00_df_appl['arrived'] = per_00_df_appl['arrived'].map({1: True, 0: False})
    per_10_df_appl['arrived'] = per_10_df_appl['arrived'].map({1: True, 0: False})

    per_00_df_appl.set_index(['module'], inplace=True)
    per_10_df_appl.set_index(['module'], inplace=True)

    # build speed data
    # per10_speedpath = os.path.abspath('.\PER1.0_speed_data')
    # per00_speedpath = os.path.abspath('.\PER0.0_speed_data')
    #
    # per10_df_speed = pandas.DataFrame(columns=['module', 'mean_speed', 'std_speed'])
    # per00_df_speed = pandas.DataFrame(columns=['module', 'mean_speed', 'std_speed'])
    #
    # for file in os.listdir(per10_speedpath):
    #     stats = get_speed_stats(os.path.join(per10_speedpath, file))
    #     per10_df_speed.loc[len(per10_df_speed)] = stats
    #
    # for file in os.listdir(per00_speedpath):
    #     stats = get_speed_stats(os.path.join(per00_speedpath, file))
    #     per00_df_speed.loc[len(per00_df_speed)] = stats
    #
    # per00_df_speed.set_index(['module'], inplace=True)
    # per10_df_speed.set_index(['module'], inplace=True)

    per00_df_speed, per10_df_speed = pandas.DataFrame(), pandas.DataFrame()
    per00_df_speed['mean_speed'] = per_00_df_mobility['total_dist'] / per_00_df_mobility['total_time']
    per10_df_speed['mean_speed'] = per_10_df_mobility['total_dist'] / per_10_df_mobility['total_time']

    # join all tables
    per00_df = pandas.merge(per_00_df_mobility, per_00_df_appl, left_index=True, right_index=True, how='outer')
    per10_df = pandas.merge(per_10_df_mobility, per_10_df_appl, left_index=True, right_index=True, how='outer')

    per00_df = pandas.merge(per00_df, per00_df_speed, left_index=True, right_index=True, how='outer')
    per10_df = pandas.merge(per10_df, per10_df_speed, left_index=True, right_index=True, how='outer')

    per00_df.to_csv('per0.0_total_stats.csv')
    per10_df.to_csv('per1.0_total_stats.csv')


def analysis_arrived_vhc():
    per00_df = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10_df = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])

    per00_arrived_cnt = per00_df['arrived'].sum()
    per10_arrived_cnt = per10_df['arrived'].sum()

    objects = ('PER 0.0', 'PER 1.0')
    x_ax = numpy.arange(len(objects))

    pyplot.bar(x_ax, [per00_arrived_cnt, per10_arrived_cnt])
    pyplot.yscale('linear')
    pyplot.yticks([per00_arrived_cnt, per10_arrived_cnt])
    pyplot.xticks(x_ax, objects)
    pyplot.ylabel('N° de vehículos que alcanzaron su destino')
    pyplot.title('PER 0.0 vs PER 1.0: N° de vehículos que alcanzaron su destino en el tiempo de simulación.')
    pyplot.show()

def analysis_speed():
    per00_df = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10_df = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])

    # filter
    per00_filtered = per00_df.loc[per00_df['arrived']]
    per10_filtered = per10_df.loc[per10_df['arrived']]

    bins = numpy.linspace(0, per00_df['mean_speed'].max(), 50)

    pyplot.hist(per00_filtered['mean_speed'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_filtered['mean_speed'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de velocidades promedio')
    pyplot.show()


def analysis_distance():
    per00_df = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10_df = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])

    # filter
    per00_filtered = per00_df.loc[per00_df['arrived']]
    per10_filtered = per10_df.loc[per10_df['arrived']]

    bins = numpy.linspace(0, per00_df['total_dist'].max(), 50)

    pyplot.hist(per00_filtered['total_dist'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_filtered['total_dist'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de distancias totales')
    pyplot.show()


def analysis_time():
    per00_df = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10_df = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])

    # filter
    per00_filtered = per00_df.loc[per00_df['arrived']]
    per10_filtered = per10_df.loc[per10_df['arrived']]

    bins = numpy.linspace(0, 1000, 50)

    pyplot.hist(per00_filtered['total_time'], bins, alpha=0.75, label='PER 0.0', normed=True)
    pyplot.hist(per10_filtered['total_time'], bins, alpha=0.75, label='PER 1.0', normed=True)
    pyplot.legend(loc='upper right')
    pyplot.title('Histograma normalizado de tiempos totales')
    pyplot.show()


if __name__ == '__main__':
    #analysis_time()
    analysis_speed()

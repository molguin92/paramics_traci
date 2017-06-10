# python 3
import matplotlib

matplotlib.use('pgf')
pgf_with_pdflatex = {
    "pgf.texsystem": "pdflatex",
    "pgf.preamble": [
         r"\usepackage[utf8x]{inputenc}",
         r"\usepackage[T1]{fontenc}",
         r"\usepackage{cmbright}",
         ]
}
matplotlib.rcParams.update(pgf_with_pdflatex)

import pandas
import re

import numpy
from matplotlib import pyplot

matplotlib.style.use('ggplot')
pyplot.interactive(False)

def to_min_secs(x, pos):
    x = int(x)
    minutes = x // 60
    seconds = x % 60
    return '{:02d}:{:02d}'.format(minutes, seconds)


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


def build_dataframe_case(case):
    # mobility data
    mobility_columns = ['module', 'max_speed', 'min_speed', 'start_time', 'stop_time',
                        'total_co2', 'total_dist', 'total_time']

    case_df_mobility = pandas.read_csv(case + '_stats_veinsmobility.csv')
    case_df_mobility.columns = mobility_columns

    mobility_search_re = 'ProvidenciaExampleScenario.(.+?).veinsmobility'
    case_df_mobility['module'] = case_df_mobility['module'].map(lambda x: re.search(mobility_search_re, x).group(1))

    case_df_mobility.set_index(['module'], inplace=True)

    # appl data (sent warnings, arrived at dest)
    appl_columns = ['module', 'arrived', 'rcvd_warnings', 'sent_warnings']
    case_df_appl = pandas.read_csv(case + '_stats_appl.csv')
    case_df_appl.columns = appl_columns

    appl_search_re = 'ProvidenciaExampleScenario.(.+?).appl'
    case_df_appl['module'] = case_df_appl['module'].map(lambda x: re.search(appl_search_re, x).group(1))
    case_df_appl['arrived'] = case_df_appl['arrived'].map({1: True, 0: False})

    case_df_appl.set_index(['module'], inplace=True)

    case_df_speed = pandas.DataFrame()
    case_df_speed['mean_speed'] = case_df_mobility['total_dist'] / case_df_mobility['total_time']

    # join all tables
    case_df = pandas.merge(case_df_mobility, case_df_appl, left_index=True, right_index=True, how='outer')
    case_df = pandas.merge(case_df, case_df_speed, left_index=True, right_index=True, how='outer')

    return case_df


def buid_csv():
    for case in ['per0.0', 'per1.0', 'base_case', 'per0.5', 'per0.75', 'per0.25']:
        df = build_dataframe_case(case)
        df.to_csv(case + '_total_stats.csv')


def analysis_arrived_vhc():
    per00 = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10 = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])
    base = pandas.read_csv('base_case_total_stats.csv').set_index(['module'])
    per05 = pandas.read_csv('per0.5_total_stats.csv').set_index(['module'])
    per075 = pandas.read_csv('per0.75_total_stats.csv').set_index(['module'])
    per025 = pandas.read_csv('per0.25_total_stats.csv').set_index(['module'])

    base_arrived_cnt = base['arrived'].sum()
    per00_arrived_cnt = per00['arrived'].sum()
    per10_arrived_cnt = per10['arrived'].sum()
    per05_arrived_cnt = per05['arrived'].sum()
    per075_arrived_cnt = per075['arrived'].sum()
    per025_arrived_cnt = per025['arrived'].sum()

    #objects = ('Caso Base', 'PER 0.0', 'PER 0.25', 'PER 0.5', 'PER 0.75', 'PER 1.0')
    objects = ('Caso Base', 'PER 0.0', 'PER 1.0')
    x_ax = numpy.arange(len(objects))

    #bars = [base_arrived_cnt, per00_arrived_cnt, per025_arrived_cnt,
    #        per05_arrived_cnt, per075_arrived_cnt, per10_arrived_cnt]

    bars = [base_arrived_cnt, per00_arrived_cnt, per10_arrived_cnt]

    pyplot.bar(x_ax, bars)
    #pyplot.yscale('log')
    pyplot.yticks(bars)
    pyplot.xticks(x_ax, objects)

    for a, b in zip(x_ax, bars):
        pyplot.text(a, b, str(b))

    #pyplot.ylabel('N° de vehículos que alcanzaron su destino')
    pyplot.title('N° de vehículos que alcanzaron su destino (escala log)')
    pyplot.show()


def analysis_speed():
    per00 = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10 = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])
    base = pandas.read_csv('base_case_total_stats.csv').set_index(['module'])
    per05 = pandas.read_csv('per0.5_total_stats.csv').set_index(['module'])
    per075 = pandas.read_csv('per0.75_total_stats.csv').set_index(['module'])
    per025 = pandas.read_csv('per0.25_total_stats.csv').set_index(['module'])

    y = [base.loc[base['arrived'] == False]['mean_speed'].mean(),
         per00.loc[per00['arrived'] == False]['mean_speed'].mean(),
         per025.loc[per025['arrived'] == False]['mean_speed'].mean(),
         per05.loc[per05['arrived'] == False]['mean_speed'].mean(),
         per075.loc[per075['arrived'] == False]['mean_speed'].mean(),
         per10.loc[per10['arrived'] == False]['mean_speed'].mean()]

    objects = ('Caso Base', 'PER 0.0', 'PER 0.25', 'PER 0.5', 'PER 0.75', 'PER 1.0')
    x = numpy.arange(len(objects))
    pyplot.bar(x, y)
    pyplot.yscale('log')
    #pyplot.yticks(y)
    pyplot.xticks(x, objects)
    pyplot.ylabel('Velocidad m/s')
    pyplot.title('Velocidades promedio de vehículos que NO alcanzaron su destino.')

    for a, b in zip(x, y):
        pyplot.text(a, b, str(b))

    pyplot.show()


def analysis_distance():
    per00 = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10 = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])
    base = pandas.read_csv('base_case_total_stats.csv').set_index(['module'])
    per05 = pandas.read_csv('per0.5_total_stats.csv').set_index(['module'])
    per075 = pandas.read_csv('per0.75_total_stats.csv').set_index(['module'])
    per025 = pandas.read_csv('per0.25_total_stats.csv').set_index(['module'])

    # filter
    data = [base.loc[base['arrived'] != True]['total_dist'], per00.loc[per00['arrived'] != True]['total_dist'], per025.loc[per025['arrived'] != True]['total_dist'],
            per05.loc[per05['arrived'] != True]['total_dist'], per075.loc[per075['arrived'] != True]['total_dist'], per10.loc[per10['arrived'] != True]['total_dist']]

    labels = ['Caso Base', 'PER 0.0', 'PER 0.25',
              'PER 0.5', 'PER 0.75', 'PER 1.0']

    bins = numpy.linspace(0, base['total_dist'].max(), 50)

    fig, axes = pyplot.subplots(nrows=2, ncols=3, sharey=True)
    fig.suptitle("Frecuencia relativa de distancias recorridas - autos que NO llegaron a su destino.")
    for idx, ax in enumerate(axes.ravel()):
        x, y, _ = ax.hist(data[idx], bins, label=labels[idx], normed=True)
        pyplot.setp(ax.get_yticklabels(), visible=True)
        ax.legend(loc='upper right')
    pyplot.show()


def analysis_time():
    per00 = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10 = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])
    base = pandas.read_csv('base_case_total_stats.csv').set_index(['module'])
    per05 = pandas.read_csv('per0.5_total_stats.csv').set_index(['module'])
    per075 = pandas.read_csv('per0.75_total_stats.csv').set_index(['module'])
    per025 = pandas.read_csv('per0.25_total_stats.csv').set_index(['module'])

    # filter
    data = [base.loc[base['arrived'] == False]['total_time'], per00.loc[per00['arrived'] == False]['total_time'],
            per025.loc[per025['arrived'] == False]['total_time'],
            per05.loc[per05['arrived'] == False]['total_time'], per075.loc[per075['arrived'] == False]['total_time'],
            per10.loc[per10['arrived'] == False]['total_time']]

    labels = ['Caso Base', 'PER 0.0', 'PER 0.25',
              'PER 0.5', 'PER 0.75', 'PER 1.0']

    bins = numpy.linspace(0, base['total_dist'].max(), 50)

    fig, axes = pyplot.subplots(nrows=2, ncols=3)
    for idx, ax in enumerate(axes.ravel()):
        ax.hist(data[idx], bins, label=labels[idx], normed=True)
        ax.legend(loc='upper right')

    pyplot.show()


def per00_vs_per10_distancetime():
    per00 = pandas.read_csv('per0.0_total_stats.csv').set_index(['module'])
    per10 = pandas.read_csv('per1.0_total_stats.csv').set_index(['module'])
    base = pandas.read_csv('base_case_total_stats.csv').set_index(['module'])

    per00 = per00.loc[per00['arrived']]

    index1 = per00.index
    index2 = per10.index

    per10 = per10[index2.isin(index1)]
    index2 = per10.index
    per00 = per00[index1.isin(index2)]

    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    ax.scatter(per00['total_dist'], per00['total_time'], marker='o', s=12, alpha=0.75, label='PER 0.0', color='#ff0000')
    ax.scatter(per10['total_dist'], per10['total_time'], marker='o', s=12, alpha=0.75, label='PER 1.0', color='#33cc22')
    ax.legend(loc='upper left')

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.yaxis.set_major_formatter(formatter)

    pyplot.xlabel('Distancia Total [m]')
    pyplot.ylabel('Tiempo Total [MM:SS]')

    pyplot.savefig('per00per10_timedistance.pgf')
    #pyplot.show()


if __name__ == '__main__':
    # buid_csv()
    #analysis_arrived_vhc()
    #analysis_distance()
    #analysis_time()
    # analysis_speed()
    per00_vs_per10_distancetime()

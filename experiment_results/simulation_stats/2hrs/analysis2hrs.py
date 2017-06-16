import matplotlib

# matplotlib.use('pgf')
# pgf_with_pdflatex = {
#     "pgf.texsystem": "pdflatex",
#     "pgf.preamble": [
#          r"\usepackage[utf8x]{inputenc}",
#          r"\usepackage[T1]{fontenc}",
#          r"\usepackage{cmbright}",
#          ]
# }
# matplotlib.rcParams.update(pgf_with_pdflatex)

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
    for case in ['per00', 'per10']:
        df = build_dataframe_case(case)
        df.to_csv(case + '_total_stats.csv')

def arrived():
    per00 = pandas.read_csv('per00_total_stats.csv')
    per10 = pandas.read_csv('per10_total_stats.csv')

    per00_arr = per00['arrived'].sum()
    per10_arr = per10['arrived'].sum()

    print(per00_arr, per10_arr)

def per00_vs_per10_distancetime():
    per00 = pandas.read_csv('per00_total_stats.csv')
    per10 = pandas.read_csv('per10_total_stats.csv')

    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    ax.scatter(per00['total_dist'], per00['total_time'], marker='o', s=4, alpha=0.75, label='PER 0.0', color='#ff0000')
    ax.scatter(per10['total_dist'], per10['total_time'], marker='o', s=4, alpha=0.75, label='PER 1.0', color='#33cc22')
    ax.legend(loc='upper right')

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.yaxis.set_major_formatter(formatter)

    pyplot.xlabel('Distancia Total [m]')
    pyplot.ylabel('Tiempo Total [MM:SS]')

    #pyplot.savefig('per00per10_timedistance.pgf')
    pyplot.show()

def per00_vs_per10_co2distance():
    per00 = pandas.read_csv('per00_total_stats.csv')
    per10 = pandas.read_csv('per10_total_stats.csv')

    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    ax.scatter(per00['total_dist'], per00['total_co2'], marker='o', s=4, alpha=0.75, label='PER 0.0', color='#ff0000')
    ax.scatter(per10['total_dist'], per10['total_co2'], marker='o', s=4, alpha=0.75, label='PER 1.0', color='#33cc22')
    ax.legend(loc='upper right')


    pyplot.ylabel('CO2 Total')
    pyplot.xlabel('Distancia Total')

    #pyplot.savefig('per00per10_co2.pgf')
    pyplot.show()


if __name__ == '__main__':
    arrived()
    per00_vs_per10_distancetime()
    per00_vs_per10_co2distance()

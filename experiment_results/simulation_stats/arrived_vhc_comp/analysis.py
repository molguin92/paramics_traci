# python 3
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
from matplotlib2tikz import save as tikz_save
import numpy
from matplotlib import pyplot

matplotlib.style.use('ggplot')
pyplot.interactive(False)

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
    for case in ['20DemandPER00', '20DemandPER10', '60DemandPER00', '60DemandPER10', '50DemandPER00', '50DemandPER10']:
        df = build_dataframe_case(case)
        df.to_csv(case + '_total_stats.csv')


def comp_graph():
    D20PER00 = pandas.read_csv('20DemandPER00_total_stats.csv')
    D20PER10 = pandas.read_csv('20DemandPER10_total_stats.csv')
    D50PER00 = pandas.read_csv('50DemandPER00_total_stats.csv')
    D50PER10 = pandas.read_csv('50DemandPER10_total_stats.csv')
    D100PER00 = pandas.read_csv('100DemandPER00_total_stats.csv')
    D100PER10 = pandas.read_csv('100DemandPER10_total_stats.csv')

    per00 = (D20PER00['arrived'].sum(), D50PER00['arrived'].sum(), D100PER00['arrived'].sum())
    per10 = (D20PER10['arrived'].sum(), D50PER10['arrived'].sum(), D100PER10['arrived'].sum())

    x_ax = numpy.arange(len(per00))
    w = 0.20

    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)
    ax.set_yscale('linear')

    bar1 = ax.bar(x_ax, per10, w, color='red', label='Sin comunicacion')
    bar2 = ax.bar(x_ax + w, per00, w, color='blue', label='Con comunicacion')

    ax.set_xlabel('Factor de Demanda')
    ax.set_xticks(x_ax + w / 2)
    ax.set_xticklabels(('20', '50', '100'))

    ax.legend(loc='upper left')
    #ax.legend((bar1[0], bar2[0]), ('Sin comunicacion', 'Con comunicacion'))

    def autolabel(rects):
        """
        Attach a text label above each bar displaying its height
        """
        for rect in rects:
            height = rect.get_height()
            ax.text(rect.get_x() + rect.get_width() / 2., height + 40,
                    '%d' % int(height),
                    ha='center', va='bottom')

    autolabel(bar1)
    autolabel(bar2)

    tikz_save('arrived_comp.tex',
              figureheight='\\figureheight',
              figurewidth='\\figurewidth')

    pyplot.show()



if __name__ == '__main__':
    comp_graph()

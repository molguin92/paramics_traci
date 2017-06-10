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
from matplotlib import pyplot
from matplotlib2tikz import save as tikz_save
from psutil import virtual_memory
import numpy

matplotlib.style.use('ggplot')
pyplot.interactive(False)

def to_min_secs(x, pos):
    x = int(x)
    minutes = x // 60
    seconds = x % 60
    return '{:02d}:{:02d}'.format(minutes, seconds)

def system_performance():
    total_RAM_mb = virtual_memory().total / (1024 * 1024)

    c = ['timestamp', 'escrituras hdd', 'lecturas hdd',
         'total I/o', '% freq procesador', '% rendimiento procesador',
         '% de uso procesador', '% tiempo procesador', 'MB disponibles RAM']
    df = pandas.read_csv('SystemStats.csv', encoding='ISO-8859-1')
    df.columns = c


    df['timestamp'] = pandas.to_datetime(df['timestamp'])
    starttime = df['timestamp'].min()
    df['delta_t'] = starttime
    temp = (df['timestamp'] - df['delta_t']).map(lambda x: int(round(x.total_seconds())))
    df['delta_t'] = temp

    df['MB disponibles RAM'] = pandas.to_numeric(df['MB disponibles RAM'])
    df['% uso RAM'] = df['MB disponibles RAM'].map(lambda free: ((total_RAM_mb - free) / total_RAM_mb) * 100)

    df['% de uso procesador']= pandas.to_numeric(df['% de uso procesador'], errors='coerce')

    fig, ax = pyplot.subplots()
    ax.plot(df['delta_t'], df['% de uso procesador'], label='% uso procesador')
    ax.plot(df['delta_t'], df['% uso RAM'], label='% uso RAM')
    ax.legend(loc='lower center')
    pyplot.xlabel('Tiempo (MM:SS)')
    pyplot.ylabel('Porcentaje')
    pyplot.xlim([0, 1600])
    pyplot.ylim([0, 60])

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.xaxis.set_major_formatter(formatter)

    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    #tikz_save('system_performance.tikz',
     #         figureheight='\\figureheight',
     #         figurewidth='\\figurewidth')
    # pyplot.savefig('system_performance.pgf')
    pyplot.show()

def system_io():
    total_RAM_mb = virtual_memory().total / (1024 * 1024)

    c = ['timestamp', 'escrituras hdd', 'lecturas hdd',
         'total I/O', '% freq procesador', '% rendimiento procesador',
         '% de uso procesador', '% tiempo procesador', 'MB disponibles RAM']
    df = pandas.read_csv('SystemStats.csv', encoding='ISO-8859-1')
    df.columns = c


    df['timestamp'] = pandas.to_datetime(df['timestamp'])
    starttime = df['timestamp'].min()
    df['delta_t'] = starttime
    temp = (df['timestamp'] - df['delta_t']).map(lambda x: int(round(x.total_seconds())))
    df['delta_t'] = temp

    df['MB disponibles RAM'] = pandas.to_numeric(df['MB disponibles RAM'])
    df['% uso RAM'] = df['MB disponibles RAM'].map(lambda free: ((total_RAM_mb - free) / total_RAM_mb) * 100)

    df['% de uso procesador']= pandas.to_numeric(df['% de uso procesador'], errors='coerce')
    df['total I/O'] = pandas.to_numeric(df['total I/O'], errors='coerce')

    print(df)

    fig, ax = pyplot.subplots()
    ax.plot(df['delta_t'], df['total I/O'], label='Operaciones I/O en disco por segundo')
    ax.legend(loc='upper left')
    pyplot.xlabel('Tiempo (MM:SS)')
    pyplot.ylabel('Operaciones I/O por segundo')
    pyplot.xlim([0, 1600])

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.xaxis.set_major_formatter(formatter)

    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    #tikz_save('system_io.tikz',
    #          figureheight='\\figureheight',
    #          figurewidth='\\figurewidth')
    #pyplot.savefig('system_io.pgf')
    pyplot.show()

def vehicles_vs_time_evolution():
    cols = ['time', 'nvehicles']
    df = pandas.read_csv('vehicles_vs_time/nvehicles.csv')
    df.columns = cols

    cols2 = ['time', 'realtime']
    df2 = pandas.read_csv('vehicles_vs_time/realtime.csv')
    df2.columns = cols2

    df['realtime'] = df2['realtime']
    df['time'] = df['time'].map(lambda x: x - 27000)
    print(df)

    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    ax.plot(df['realtime'], df['nvehicles'], '.-', label='Tiempo Real')
    ax.plot(df['time'], df['nvehicles'], '.-', label='Tiempo Simulado')

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.xaxis.set_major_formatter(formatter)

    pyplot.ylabel('Número de Vehículos en Simulación')
    pyplot.xlabel('Tiempo [MM:SS]')
    pyplot.legend(loc='lower right')
    pyplot.savefig('timevsvehicles_evolution.pgf')
    #pyplot.show()


def vehicles_vs_time():
    cols = ['nvhcs', 't', 'demand', 'runID']
    df = pandas.read_csv('NVeh_vs_T.csv', encoding='ISO-8859-1', sep=';')
    df.columns = cols

    df['nvhcs'] = pandas.to_numeric(df['nvhcs'])
    df['t'] = pandas.to_numeric(df['t'])
    df['runID'] = pandas.to_numeric(df['runID'])

    df['demand'] = df['demand'].map(lambda x: float(x.strip('%'))/100)


    df100 = df.loc[df['demand'] == 1.00]
    df75 = df.loc[df['demand'] == 0.75]
    df50 = df.loc[df['demand'] == 0.50]
    df25 = df.loc[df['demand'] == 0.25]

    mean_df = pandas.DataFrame(columns=['demand', 'mean_vhcs', 'mean_time'])
    mean_df.loc[0] = [1.00, df100['nvhcs'].mean(), df100['t'].mean()]
    mean_df.loc[1] = [0.75, df75['nvhcs'].mean(), df75['t'].mean()]
    mean_df.loc[2] = [0.50, df50['nvhcs'].mean(), df50['t'].mean()]
    mean_df.loc[3] = [0.25, df25['nvhcs'].mean(), df25['t'].mean()]

    # from this point onward, plot
    fig, ax = pyplot.subplots()
    ax.set_facecolor('white')
    ax.grid(color='#a1a1a1', linestyle='-', alpha=0.1)

    pyplot.xlim([df['nvhcs'].min() - 50, df['nvhcs'].max() + 50])
    pyplot.ylim(0, df['t'].max() + 120)

    yticks_mins = numpy.arange(0, df['t'].max() + 120, 120)
    yticks_10secs = numpy.arange(0, df['t'].max() + 120, 60)

    xticks = numpy.arange(200, 1500, 100)
    xticks_minor = numpy.arange(150, 1500, 10)

    ax.set_yticks(yticks_mins)
    ax.set_yticks(yticks_10secs, minor=True)

    ax.set_xticks(xticks)
    ax.set_xticks(xticks_minor, minor=True)

    # trendline
    z = numpy.polyfit(df['nvhcs'], df['t'], 2)
    p = numpy.poly1d(z)
    nx = range(0, int(df['nvhcs'].max()) + 200)

    print(p)

    ax.plot(nx, p(nx), '-.', alpha=0.3, label='Ajuste polinomial', color='#F06449')

    # scatter
    ax.plot(df100['nvhcs'], df100['t'], 'o', color='#17BEBB', label='Factor de demanda 100%')
    ax.plot(df75['nvhcs'], df75['t'], 'o', color='#EF2D56', label='Factor de demanda 75%')
    ax.plot(df50['nvhcs'], df50['t'], 'o', color='#8CD867', label='Factor de demanda 50%')
    ax.plot(df25['nvhcs'], df25['t'], 'o', color='#2F243A', label='Factor de demanda 25%')
    ax.legend(loc='upper left')
    pyplot.ylabel('Tiempo (MM:SS)')

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.yaxis.set_major_formatter(formatter)

    pyplot.xlabel('Cantidad promedio vehículos en simulación')
    # pyplot.title('Scatterplot: Cantidad promedio de vehículos vs duración en tiempo real de simulación')
    #pyplot.savefig('n_vhcs_vs_time.pgf')
    pyplot.show()

    #tikz_save('n_vhcs_vs_time.tikz',
     #         figureheight='\\figureheight',
      #        figurewidth='\\figurewidth')


if __name__ == '__main__':
    #system_performance()
    vehicles_vs_time_evolution()
    #system_io()

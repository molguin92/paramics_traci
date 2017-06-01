import matplotlib
import pandas
from matplotlib import pyplot
from psutil import virtual_memory
import numpy

matplotlib.style.use('ggplot')
pyplot.interactive(False)


def to_min_secs(x, pos):
    x = int(x)
    minutes = x // 60
    seconds = x % 60
    return '{}:{}'.format(minutes, seconds)

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

    print(df)

    df['% de uso procesador']= pandas.to_numeric(df['% de uso procesador'], errors='coerce')

    fig, ax = pyplot.subplots()
    ax.plot(df['delta_t'], df['% de uso procesador'], label='% uso procesador')
    ax.plot(df['delta_t'], df['% uso RAM'], label='% uso RAM')
    ax.legend(loc='lower center')
    pyplot.xlabel('tiempo (m:s)')
    pyplot.ylabel('porcentaje %')
    pyplot.xlim([0, 1600])
    pyplot.ylim([0, 60])

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.xaxis.set_major_formatter(formatter)

    pyplot.show()


def vehicles_vs_time():
    cols = ['nvhcs', 't', 'demand', 'runID']
    df = pandas.read_csv('NVeh_vs_T.csv', encoding='ISO-8859-1', sep=';')
    df.columns = cols

    df['nvhcs'] = pandas.to_numeric(df['nvhcs'])
    df['t'] = pandas.to_numeric(df['nvhcs'])
    df['runID'] = pandas.to_numeric(df['runID'])

    df['demand'] = df['demand'].map(lambda x: float(x.strip('%'))/100)

    print(df)

    fig, ax = pyplot.subplots()

    # trendline
    z = numpy.polyfit(df['nvhcs'], df['t'], 1)
    p = numpy.poly1d(z)
    nx = range(0, int(df['nvhcs'].max()) + 200)
    ax.plot(nx, p(nx), 'b--', alpha=0.1, label='Ajuste lineal')

    # scatter
    ax.plot(df.loc[df['demand'] == 1.00]['nvhcs'], df.loc[df['demand'] == 1.00]['t'], 'or', label='Carga 100%')
    ax.plot(df.loc[df['demand'] == 0.75]['nvhcs'], df.loc[df['demand'] == 0.75]['t'], 'oy', label='Carga 75%')
    ax.plot(df.loc[df['demand'] == 0.50]['nvhcs'], df.loc[df['demand'] == 0.50]['t'], 'ob', label='Carga 50%')
    ax.plot(df.loc[df['demand'] == 0.25]['nvhcs'], df.loc[df['demand'] == 0.25]['t'], 'og', label='Carga 25%')
    ax.legend(loc='upper left')
    pyplot.ylabel('Tiempo (MM:SS)')

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.yaxis.set_major_formatter(formatter)

    pyplot.xlabel('Cantidad promedio vehículos en simulación')
    pyplot.title('Scatterplot: Cantidad promedio de vehículos vs duración en tiempo real de simulación')

    pyplot.xlim([df['nvhcs'].min()-50, df['nvhcs'].max()+50])


    pyplot.show()


if __name__ == '__main__':
    #system_performance()
    vehicles_vs_time()

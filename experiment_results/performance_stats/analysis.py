import matplotlib
import pandas
from matplotlib import pyplot
from psutil import virtual_memory

matplotlib.style.use('ggplot')
pyplot.interactive(False)
total_RAM_mb = virtual_memory().total / (1024 * 1024)

def read_data():
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

    def to_min_secs(x, pos):
        x = int(x)
        minutes = x // 60
        seconds = x % 60
        return '{}:{}'.format(minutes, seconds)

    formatter = matplotlib.ticker.FuncFormatter(to_min_secs)
    ax.xaxis.set_major_formatter(formatter)

    pyplot.show()


if __name__ == '__main__':
    read_data()

#!/usr/bin/env python

# -------- checking the environment --------

if 1/3 == 0:
    print('The program requires Python 3.x rather than 2.x and older')
    exit(1)

if __name__ != '__main__':
    print('The program cannot be used as a library, use it as Python application program')
    exit(2)

try:
    import matplotlib.pyplot as plot
except:
    print('There is no library imported as "import matplotlib.pyplot"')
    exit(3)

try:
    import numpy
except:
    print('There is no library imported as "import numpy"')
    exit(4)

# -------- the main part --------

def addNewComplexityLine(
        *
        , independentPlot
        , lengthsOfArrays
        , executionsTimes
        , label
        , color
        , marker
        , lineStyle
        , lineWidth
    ):
    independentPlot.plot(
          lengthsOfArrays
        , executionsTimes
        , label = label
        , color = color
        , marker = marker
        , linestyle = lineStyle
        , linewidth = lineWidth
    )

def newLine(executionsTimes, label, color, marker, lineStyle, lineWidth):
    return {
          'executions_times': executionsTimes
        , 'label': label
        , 'color': color
        , 'marker': marker
        , 'line_style': lineStyle
        , 'line_width': lineWidth
    }

def generateTitle(algorithmName, sequenceName):
    return sequenceName + ' ' + algorithmName

def newRangeForGrid(from_, to, step):
    return numpy.arange(from_, to, step)

def newPlot(
        *
        , majorXTicks
        , minorXTicks
        , majorYTicks
        , minorYTicks
    ):

    gridPlot = plot.figure().add_subplot(1, 1, 1)

    gridPlot.set_xticks(majorXTicks)
    gridPlot.set_xticks(minorXTicks, minor = True)
    gridPlot.set_yticks(majorYTicks)
    gridPlot.set_yticks(minorYTicks, minor = True)

    gridPlot.grid(which = 'both')

    gridPlot.grid(which = 'major', alpha = 0.6)
    gridPlot.grid(which = 'minor', alpha = 0.2)

    return gridPlot

def newAsymtot(start, end, step, function):
    valuesX = []
    valuesY = []

    currentX = start
    while currentX <= end:
        valuesX.append(currentX)
        valuesY.append(function(currentX))

        currentX += step

    return {
          'values_x': valuesX
        , 'values_y': valuesY
    }

# -------- the configuration --------

lengthsOfArrays = [
       100
    ,  250
    ,  500
    , 1000
]

# ===========================================

lupItersCount = [
         378650
    ,   5490375
    ,  42793250
    , 337836500
]
ghItersCount = [
         191900
    ,   2761000
    ,  21459500
    , 169169000
]
rmItersCount = [
         358350
    ,   5364625
    ,  42291750
    , 335833500
]


# ===========================================

lupColor = 'green'
ghColor = 'red'
rmColor = 'cyan'

lupMarker = 'x'
ghMarker = '+'
rmMarker = 'o'

lupLineStyle = 'dotted'
ghLineStyle = 'solid'
rmLineStyle = 'dashline'

lupName = 'LUP-метод'
ghName = 'Метод Гауса-Холецького'
rmName = 'Метод обертання'

labelX = 'Кількість рівнянь СЛАР'
labelY = 'Кількість ітерацій'

legendFontSize = 8
legendLocation = 'upper left'

maxYOnPlot = max([*lupItersCount, *ghItersCount, *rmItersCount])

# -------- the meta-configuraiton --------

bubbleSortLines = [
    newLine(lupItersCount, lupName, lupColor, lupMarker, lupLineStyle, 1)
]

modBubbleSortLines = [
    newLine(ghItersCount, ghName, ghColor, lupMarker, lupLineStyle, 1)
]

insSortLines = [
    newLine(rmItersCount, rmName, rmColor, lupMarker, lupLineStyle, 1)
]

maxXOnPlot = max(lengthsOfArrays)

# -------- randering --------

plot_ = newPlot(
      majorXTicks = newRangeForGrid(0, maxXOnPlot, maxXOnPlot / 10)
    , minorXTicks = newRangeForGrid(0, maxXOnPlot, maxXOnPlot / 10 / 5)
    , majorYTicks = newRangeForGrid(0, maxYOnPlot, maxYOnPlot / 10)
    , minorYTicks = newRangeForGrid(0, maxYOnPlot, maxYOnPlot / 10 / 5)
)

for line in [*bubbleSortLines, *modBubbleSortLines, *insSortLines]:
    addNewComplexityLine(
        independentPlot = plot_
        , lengthsOfArrays = lengthsOfArrays
        , executionsTimes = line['executions_times']
        , label = line['label']
        , color = line['color']
        , marker = line['marker']
        , lineStyle = line['line_style']
        , lineWidth = line['line_width']
    )

plot_.set_xlabel(labelX)
plot_.set_ylabel(labelY)

plot_.legend(loc = legendLocation, prop = { 'size': legendFontSize })

plot.show()

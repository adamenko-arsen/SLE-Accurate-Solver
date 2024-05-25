# About

This program solves systems of linear equations (SLE) by the different accurate methods sized from 1 to 10 inclusively. This project has given to me and chose by me as a course work (KPI, 1st course, 2nd term).

# Main

## Method

1. LUP-method
1. Gauss-Holetskiy method
1. Rotation method

## Features

- the opportunity to try to solve any inputed system of linear equations
- the opportunity to choose a solving method freely
- The opportunity to output the solve of SLE
- the opportunity to output the 2D graphical representation of two linear equations and their solve
- the ability to warn an user about his/her incorrectly typed entries
- the ability to warn an user about impossibility of solving inputed SLE

## System requirements

<table>
  <th>
    <td>Minimal</td><td>Recommended</td>
  </th>
  <tr>
    <td>OS</td>
    <td>Linux (probably)</td>
    <td>ArchLinux</td>
  </tr>
  <tr>
    <td>CPU</td>
    <td colspan=2>Intel i7-6700HQ (8) @ 3.500GHz</td>
  </tr>
  <tr>
    <td>GPU</td>
    <td colspan=2>Intel HD Graphics 530</td>
  </tr>
  <tr>
    <td>RAM</td>
    <td>512MiB</td>
    <td>2GiB</td>
  </tr>
  <tr>
    <td>Monitor</td>
    <td colspan=2>1920x1080 and higher</td>
  </tr>
  <tr>
    <td>Human input devices</td>
    <td colspan=2>Keyboard and mouse</td>
  </tr>
  <tr>
    <td>Extra software</td>
    <td colspan=2>Packages: <code>base</code>, <code>base-devel</code>, <code>gtkmm3</code>; others: any popular desktop environment, d-bus support</td>
  </tr>
</table>

## Development

The project can be controlled broadly by the script `make.sh`.

To build the project, run the command below:
```sh
$ ./make.sh build
```

Then you can execute by running the command below:
```sh
$ ./make.sh run
```

Type this one to get all options:
```sh
$ ./make.sh help
```

Before the moment you want to build or run the project, read the project's environmental requirements below.

### Languages

<table>
  <tr>
    <td>Name</td> <td>Version</td> <td>Purpose</td>
  </tr>
  <tr>
    <td>C++</td> <td>C++23</td> <td>the program's main language</td>
  <tr>
  <tr>
    <td>Bash</td> <td>5.2.26</td> <td>managing the project</td>
  <tr>
</table>

### Libraries

<table>
  <tr>
    <td>Name</td> <td>Version</td> <td>Purpose</td>
  </tr>
  <tr>
    <td>GTKMM</td> <td>3.0</td> <td>for the program's simple GUI</td>
  <tr>
</table>

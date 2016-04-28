# Memory consumption
|                 | bytes per empty signal | bytes per signal | bytes per handler |
| --------------- | ---------------: | ----------------: |
| ui_signal       | ${signal.createEmpty.wigwag_ui[signal]} | ${signal.create.wigwag_ui[signal]} | ${signal.handlerSize.wigwag_ui[handler]} |
| signal          | ${signal.createEmpty.wigwag[signal]} | ${signal.create.wigwag[signal]} | ${signal.handlerSize.wigwag[handler]} |
| sigc++          | ${signal.createEmpty.sigcpp[signal]} | ${signal.create.sigcpp[signal]} | ${signal.handlerSize.sigcpp[handler]} |
| boost           | ${signal.createEmpty.boost[signal]} | ${signal.create.boost[signal]} | ${signal.handlerSize.boost[handler]} |
| boost, tracking | ${signal.createEmpty.boost_tracking[signal]} | ${signal.create.boost_tracking[signal]} | ${signal.handlerSize.boost_tracking[handler]} |
| qt5             | ${signal.createEmpty.qt5[signal]} | ${signal.create.qt5[signal]} | ${signal.handlerSize.qt5[handler]} |

# Signals performance
|                 | creating empty signal, ns | destroying empty signal, ns | destroying signal, ns |
| --------------- | ------------------------: | --------------------: | --------------------------: |
| ui_signal       | ${signal.createEmpty.wigwag_ui[create]} | ${signal.createEmpty.wigwag_ui[destroy]} | ${signal.create.wigwag_ui[destroy]} |
| signal          | ${signal.createEmpty.wigwag[create]} | ${signal.createEmpty.wigwag[destroy]} | ${signal.create.wigwag[destroy]} |
| sigc++          | ${signal.createEmpty.sigcpp[create]} | ${signal.createEmpty.sigcpp[destroy]} | ${signal.create.sigcpp[destroy]} |
| boost           | ${signal.createEmpty.boost[create]} | ${signal.createEmpty.boost[destroy]} | ${signal.create.boost[destroy]} |
| qt5             | ${signal.createEmpty.qt5[create]} | ${signal.createEmpty.qt5[destroy]} | ${signal.create.qt5[destroy]} |

# Signal handlers performance
## Invoking handlers, ns per handler
|                 |    1 |    3 |   10 |  100 | 1000 | 10000 | 100000 | 1000000 |
| --------------- | ---: | ---: | ---: | ---: | ---: | ----: | -----: | ------: |
| ui_signal       | ${signal.invoke.wigwag_ui(numSlots:1)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:3)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:10)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:100)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:1000)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:10000)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:100000)[invoke]} | ${signal.invoke.wigwag_ui(numSlots:1000000)[invoke]} |
| signal          | ${signal.invoke.wigwag(numSlots:1)[invoke]} | ${signal.invoke.wigwag(numSlots:3)[invoke]} | ${signal.invoke.wigwag(numSlots:10)[invoke]} | ${signal.invoke.wigwag(numSlots:100)[invoke]} | ${signal.invoke.wigwag(numSlots:1000)[invoke]} | ${signal.invoke.wigwag(numSlots:10000)[invoke]} | ${signal.invoke.wigwag(numSlots:100000)[invoke]} | ${signal.invoke.wigwag(numSlots:1000000)[invoke]} |
| sigc++          | ${signal.invoke.sigcpp(numSlots:1)[invoke]} | ${signal.invoke.sigcpp(numSlots:3)[invoke]} | ${signal.invoke.sigcpp(numSlots:10)[invoke]} | ${signal.invoke.sigcpp(numSlots:100)[invoke]} | ${signal.invoke.sigcpp(numSlots:1000)[invoke]} | ${signal.invoke.sigcpp(numSlots:10000)[invoke]} | ${signal.invoke.sigcpp(numSlots:100000)[invoke]} | ${signal.invoke.sigcpp(numSlots:1000000)[invoke]} |
| boost           | ${signal.invoke.boost(numSlots:1)[invoke]} | ${signal.invoke.boost(numSlots:3)[invoke]} | ${signal.invoke.boost(numSlots:10)[invoke]} | ${signal.invoke.boost(numSlots:100)[invoke]} | ${signal.invoke.boost(numSlots:1000)[invoke]} | ${signal.invoke.boost(numSlots:10000)[invoke]} | ${signal.invoke.boost(numSlots:100000)[invoke]} | ${signal.invoke.boost(numSlots:1000000)[invoke]} |
| boost, tracking | ${signal.invoke.boost_tracking(numSlots:1)[invoke]} | ${signal.invoke.boost_tracking(numSlots:3)[invoke]} | ${signal.invoke.boost_tracking(numSlots:10)[invoke]} | ${signal.invoke.boost_tracking(numSlots:100)[invoke]} | ${signal.invoke.boost_tracking(numSlots:1000)[invoke]} | ${signal.invoke.boost_tracking(numSlots:10000)[invoke]} | ${signal.invoke.boost_tracking(numSlots:100000)[invoke]} | ${signal.invoke.boost_tracking(numSlots:1000000)[invoke]} |
| qt5             | ${signal.invoke.qt5(numSlots:1)[invoke]} | ${signal.invoke.qt5(numSlots:3)[invoke]} | ${signal.invoke.qt5(numSlots:10)[invoke]} | ${signal.invoke.qt5(numSlots:100)[invoke]} | ${signal.invoke.qt5(numSlots:1000)[invoke]} | ${signal.invoke.qt5(numSlots:10000)[invoke]} | ${signal.invoke.qt5(numSlots:100000)[invoke]} | ${signal.invoke.qt5(numSlots:1000000)[invoke]} |

## Connecting handlers, ns per handler
|                 |    1 |    3 |   10 |  100 |  1000 |  10000 |
| --------------- | ---: | ---: | ---: | ---: | ----: | -----: |
| ui_signal       | ${signal.connect.wigwag_ui(numSlots:1)[connect]} | ${signal.connect.wigwag_ui(numSlots:3)[connect]} | ${signal.connect.wigwag_ui(numSlots:10)[connect]} | ${signal.connect.wigwag_ui(numSlots:100)[connect]} | ${signal.connect.wigwag_ui(numSlots:1000)[connect]} | ${signal.connect.wigwag_ui(numSlots:10000)[connect]} |
| signal          | ${signal.connect.wigwag(numSlots:1)[connect]} | ${signal.connect.wigwag(numSlots:3)[connect]} | ${signal.connect.wigwag(numSlots:10)[connect]} | ${signal.connect.wigwag(numSlots:100)[connect]} | ${signal.connect.wigwag(numSlots:1000)[connect]} | ${signal.connect.wigwag(numSlots:10000)[connect]} |
| sigc++          | ${signal.connect.sigcpp(numSlots:1)[connect]} | ${signal.connect.sigcpp(numSlots:3)[connect]} | ${signal.connect.sigcpp(numSlots:10)[connect]} | ${signal.connect.sigcpp(numSlots:100)[connect]} | ${signal.connect.sigcpp(numSlots:1000)[connect]} | ${signal.connect.sigcpp(numSlots:10000)[connect]} |
| boost           | ${signal.connect.boost(numSlots:1)[connect]} | ${signal.connect.boost(numSlots:3)[connect]} | ${signal.connect.boost(numSlots:10)[connect]} | ${signal.connect.boost(numSlots:100)[connect]} | ${signal.connect.boost(numSlots:1000)[connect]} | ${signal.connect.boost(numSlots:10000)[connect]} |
| boost, tracking | ${signal.connect.boost_tracking(numSlots:1)[connect]} | ${signal.connect.boost_tracking(numSlots:3)[connect]} | ${signal.connect.boost_tracking(numSlots:10)[connect]} | ${signal.connect.boost_tracking(numSlots:100)[connect]} | ${signal.connect.boost_tracking(numSlots:1000)[connect]} | ${signal.connect.boost_tracking(numSlots:10000)[connect]} |
| qt5             | ${signal.connect.qt5(numSlots:1)[connect]} | ${signal.connect.qt5(numSlots:3)[connect]} | ${signal.connect.qt5(numSlots:10)[connect]} | ${signal.connect.qt5(numSlots:100)[connect]} | ${signal.connect.qt5(numSlots:1000)[connect]} | ${signal.connect.qt5(numSlots:10000)[connect]} |

## Disconnecting handlers, ns per handler
|                 |    1 |    3 |   10 |  100 |  1000 |  10000 |
| --------------- | ---: | ---: | ---: | ---: | ----: | -----: |
| ui_signal       | ${signal.connect.wigwag_ui(numSlots:1)[disconnect]} | ${signal.connect.wigwag_ui(numSlots:3)[disconnect]} | ${signal.connect.wigwag_ui(numSlots:10)[disconnect]} | ${signal.connect.wigwag_ui(numSlots:100)[disconnect]} | ${signal.connect.wigwag_ui(numSlots:1000)[disconnect]} | ${signal.connect.wigwag_ui(numSlots:10000)[disconnect]} |
| signal          | ${signal.connect.wigwag(numSlots:1)[disconnect]} | ${signal.connect.wigwag(numSlots:3)[disconnect]} | ${signal.connect.wigwag(numSlots:10)[disconnect]} | ${signal.connect.wigwag(numSlots:100)[disconnect]} | ${signal.connect.wigwag(numSlots:1000)[disconnect]} | ${signal.connect.wigwag(numSlots:10000)[disconnect]} |
| sigc++          | ${signal.connect.sigcpp(numSlots:1)[disconnect]} | ${signal.connect.sigcpp(numSlots:3)[disconnect]} | ${signal.connect.sigcpp(numSlots:10)[disconnect]} | ${signal.connect.sigcpp(numSlots:100)[disconnect]} | ${signal.connect.sigcpp(numSlots:1000)[disconnect]} | ${signal.connect.sigcpp(numSlots:10000)[disconnect]} |
| boost           | ${signal.connect.boost(numSlots:1)[disconnect]} | ${signal.connect.boost(numSlots:3)[disconnect]} | ${signal.connect.boost(numSlots:10)[disconnect]} | ${signal.connect.boost(numSlots:100)[disconnect]} | ${signal.connect.boost(numSlots:1000)[disconnect]} | ${signal.connect.boost(numSlots:10000)[disconnect]} |
| boost, tracking | ${signal.connect.boost_tracking(numSlots:1)[disconnect]} | ${signal.connect.boost_tracking(numSlots:3)[disconnect]} | ${signal.connect.boost_tracking(numSlots:10)[disconnect]} | ${signal.connect.boost_tracking(numSlots:100)[disconnect]} | ${signal.connect.boost_tracking(numSlots:1000)[disconnect]} | ${signal.connect.boost_tracking(numSlots:10000)[disconnect]} |
| qt5             | ${signal.connect.qt5(numSlots:1)[disconnect]} | ${signal.connect.qt5(numSlots:3)[disconnect]} | ${signal.connect.qt5(numSlots:10)[disconnect]} | ${signal.connect.qt5(numSlots:100)[disconnect]} | ${signal.connect.qt5(numSlots:1000)[disconnect]} | ${signal.connect.qt5(numSlots:10000)[disconnect]} |

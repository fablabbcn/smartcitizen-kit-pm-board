# Smart Citizen Kit 2.0 PM Board

[![PRs Welcome](https://img.shields.io/badge/PRs-welcome-brightgreen.svg)]()
[![DOI](https://zenodo.org/badge/126015935.svg)](https://zenodo.org/badge/latestdoi/126015935)

![alt text](https://cdn.rawgit.com/fablabbcn/smartcitizen-kit-pm-board/2530bf9b/pm_board.png "PM and Aux Board")

Expansion board designed to expand the functionality of the Smart Citizen Kit 2.0, it has:

* 2 ports for particle [Plantower PM sensors](http://www.plantower.com/en/list/?118_1.html).
* One UART port ([grove](http://wiki.seeed.cc/Grove_System/) connector).
* One grove GPIO.
* Two grove ADC.
* In/out groove connectors to chain multiple boards to a Smart Citizen Kit 2.0.


## The core firmware (C++)

The technology used:

* Arduino Zero Core
* Platformio

#### Uploading the firmware to the board

After clonning the repository:

1. In folder ./firmware run:

  * `pio run`

2. This step will **take a while**. Once it finished plug the board over USB and run:

  * `pio run -t upload`

## Documentation

Check the [Smart Citizen documentation](https://docs.smartcitizen.me/Components/boards/PM%20Board/)

## Questions and comments

Check the [forum](https://forum.smartcitizen.me/)

## Related Smart Citizen repositories

* Smart Citizen Kit 2.1 [github.com/fablabbcn/smartcitizen-kit-21](https://github.com/fablabbcn/smartcitizen-kit-21)
* Smart Citizen Kit 2.0 [github.com/fablabbcn/smartcitizen-kit-20](https://github.com/fablabbcn/smartcitizen-kit-20)
* Platform Core API [github.com/fablabbcn/smartcitizen-api](https://github.com/fablabbcn/smartcitizen-api)
* Platform Web [github.com/fablabbcn/smartcitizen-web](https://github.com/fablabbcn/smartcitizen-web)
* Data Analysis [github.com/fablabbcn/smartcitizen-data](https://github.com/fablabbcn/smartcitizen-data)
* Platform Onboarding [github.com/fablabbcn/smartcitizen-onboarding-app](https://github.com/fablabbcn/smartcitizen-onboarding-app)
* Kit Enclosures [github.com/fablabbcn/smartcitizen-enclosures](https://github.com/fablabbcn/smartcitizen-enclosures)
* Useful software resources for communities [github.com/fablabbcn/smartcitizen-toolkit](https://github.com/fablabbcn/smartcitizen-toolkit)

## License

All the software unless stated is released under [GNU GPL v3.0](https://github.com/fablabbcn/smartcitizen-kit-20/blob/master/LICENSE) and the hardware design files under [CERN OHL v1.2](https://github.com/fablabbcn/smartcitizen-kit-20/blob/master/hardware/LICENSE)

## Credits

Smart Citizen Team 2012-ONWARDS

## Funding

This work has received funding from the European Union's Horizon 2020 research and innovation program under the grant agreement [No. 689954](https://cordis.europa.eu/project/rcn/202639_en.html)

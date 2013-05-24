Monte Carlo DPD
===============

This C program uses Monte Carlo simulations and dissipative particle dynamics (DPD) to investigate quantities like energy and pressure of a system of particles. Future plans are to add a polymer, wall and nanopore.

Installation
------------

To get latest version:

	git clone https://github.com/tdunn19/dpd

Usage
-----

To compile and run:
	
	cd dpd/src
	make opt3
	./dpd.run

Edit src/dpd.inp to change parameters.

Changelog
---------

Version 1.1 (May 23, 2013)
*	some efficiency changes:
*	a new cell list will only be generated when a particles moves into a new cell
*	new energies will only be calculated for particles in neighboring (2 neighbors deep) cells of the randomly chosen particle 

Version 1.0 (May 20, 2013)
*	employed cell list method of calculation


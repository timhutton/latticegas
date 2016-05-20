[![Build Status](https://travis-ci.org/timhutton/latticegas.svg?branch=master)](https://travis-ci.org/timhutton/latticegas)

# Lattice Gas Explorer #

Explore lattice gases, from the basics of implementation to the analysis of the flow patterns.

![http://lh5.ggpht.com/_L3XQL9bgmnM/SR9UsJfZuyI/AAAAAAAABvo/AfxEGYkSQdA/s400/eddies.jpg](http://lh5.ggpht.com/_L3XQL9bgmnM/SR9UsJfZuyI/AAAAAAAABvo/AfxEGYkSQdA/s400/eddies.jpg)
> _A visualisation of the eddies in the wake of an obstacle placed in a flow. The eddies alternate between clockwise and anti-clockwise._

### Videos: ###

[![](http://lh5.ggpht.com/_L3XQL9bgmnM/SR9UqVCwnhI/AAAAAAAABvc/erPVLpryzNg/s400/eddies.jpg)](http://picasaweb.google.com/lh/photo/KOHHeA_qKIqr08QOhzyLlg)
> _The flow behind an obstacle, showing periodic fluctuations from side to side as vortices are shed._

[![](http://lh6.ggpht.com/_L3XQL9bgmnM/SR9TIFjytEI/AAAAAAAABvU/AdooQ2Ay0fw/s800/lga_particles.jpg)](http://picasaweb.google.com/lh/photo/yWlQddL5Cp6qDAlNJnFZuw)
> _The gas particles live on a square lattice, with at most one particle to a square. Here their colour indicates the direction they are moving (N,NE,E,SE, etc.). Grey particles are at rest, but actually jiggle about on the spot because of the way PI-LGA works._

### More images: ###

[![](http://lh6.ggpht.com/_L3XQL9bgmnM/SSahIlf5hII/AAAAAAAACI8/4CtpE0S3B7M/s800/lga_obstacle_closeup.png)](http://picasaweb.google.com/lh/photo/ur9y5k3W7t-ZUG2Hy7eCyw)
> _A close-up of the lattice gas around a circular obstacle, showing the scale of the simulation. The density is visibly higher on the upstream side of the obstacle (left) than the downstream side. Vortices have formed behind the obstacle but these can't be seen without drawing flow lines._

[![](http://lh5.ggpht.com/_L3XQL9bgmnM/SSLOH4RTaZI/AAAAAAAACIs/Qp1WK7W5HqI/s400/lga_vortices.jpg)](http://picasaweb.google.com/lh/photo/oTBUJ5yExGBCoskK4Y0ukA)
> _The flowlines at a similar time to the image above, showing the flow around the obstacle and the vortices behind it. (The flowlines pass through the object but this is only because the flow average is taken over a large area.)_

[![](http://lh6.ggpht.com/_L3XQL9bgmnM/SYC0rks3hVI/AAAAAAAACMQ/BpavOWIMQiM/s800/latticegas_atoms_on_grid.png)](http://picasaweb.google.com/lh/photo/9i8rxrydIhaJcEwY1_1P9A?feat=embedwebsite)
> _Another closeup of the gas particles, showing the underlying LGA-PI grid with room for up to four particles in each square. The cells' colour and the small flow lines show the direction they are travelling in. When the same flow lines are averaged over larger areas we see the flow patterns in the images above. (And the gas is usually at a much higher density.)_

## More information: ##

  * We currently only implement "pair-interaction lattice gases" (PI-LGA). Other methods could be added, to show their differences.
  * A blog entry says a little more about the context of lattice gases and has links to papers and so on: http://ferkeltongs.livejournal.com/22630.html
  * The intention is that the program is useful as a demo, for learning about LGA. But it hasn't gone very far in this direction. Its implementation isn't particularly efficient, I'm sure you could do better.

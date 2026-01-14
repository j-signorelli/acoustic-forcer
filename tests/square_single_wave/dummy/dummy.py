#!/usr/bin/env python3

import precice
from optparse import OptionParser
import numpy as np


def main():
    
    parser=OptionParser()
    parser.add_option("-N", "--n-points", dest="N", default=10,
                        help="N_x and N_y of square [0,1]x[0,1] grid.", )
    parser.add_option("-p", "--precice-config", dest="precice_config",
                        default="../precice-config.xml", 
                        help="Read config from FILE", metavar="FILE")

    (options, args) = parser.parse_args()

    # Initialize the grid
    N = int(options.N)
    x = np.linspace(0,1,N)
    y = np.linspace(0,1,N)
    X, Y = np.meshgrid(x,y, indexing="ij")
    vertex_size = N*N
    coords = np.zeros((vertex_size, 2))
    coords[:,0] = X.flatten()
    coords[:,1] = Y.flatten()

    participant = precice.Participant("Dummy", options.precice_config, 0, 1)
    vertex_ids = participant.set_mesh_vertices("Square", coords)
    p_prime = np.zeros(vertex_size)

    participant.initialize()

    while participant.is_coupling_ongoing():
        precice_dt = participant.get_max_time_step_size()
        p_prime = participant.read_data("Square", "p'", vertex_ids, precice_dt)
        participant.advance(precice_dt)

    participant.finalize()

if __name__ == '__main__':
    main()
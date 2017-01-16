import sys
from collections import defaultdict

def parse_args():
    if len(sys.argv) != 3:
        print("Usage: python ddsg2kahip.py input.ddsg output.graph")

    input_filename = sys.argv[1]
    output_filename = sys.argv[2]
    return input_filename, output_filename

def parse_input(input_filename):
    with open(input_filename, 'r') as f:
        input_lines = f.readlines()

    if input_lines[0].strip() != 'd':
        print("Format error: first line should be 'd'")
        sys.exit(-1)

    n, m = [int(v) for v in input_lines[1].split()]

    edges = {} # (source, target) → (weight, is_forward, is_backward)
    for i in range(2, m + 2):
        line = input_lines[i]
        s, t, w, d = [int(v) for v in line.split()]

        assert 0 <= d <= 3
        if d == 3:
            continue

        if s == t:
            print("Vertex %d has a loop, skipping it" % s)
            continue

        if s > t:
            s, t = t, s
            if d != 0:
                # d is 1 or 2
                d = 3 - d

        is_forward = d in (0, 1)
        is_backward = d in (0, 2)

        if (s, t) in edges:
            if w != edges[(s, t)][0]:
                print("Edge (%d, %d) has two different weights: %d and %d; " % (s, t, edges[(s, t)][0], w), end='')
                # if (edges[(s, t)][1] and not is_forward) or (edges[(s, t)][2] and not is_backward):
                #     print("cannot proceed, exiting")
                #     exit(-1)
                print("selecting minimal")
                w = min(w, edges[(s, t)][0])
            is_forward = is_forward or edges[(s, t)][1]
            is_backward = is_backward or edges[(s, t)][2]

        edges[(s, t)] = (w, is_forward, is_backward)

    return n, m, edges

def generate_graph(edges):
    graph = defaultdict(list)
    for ((s, t), (w, is_forward, is_backward)) in edges.items():
        if not (is_forward and is_backward):
            print("Edge (%d, %d) is directed. Replacing it with an undirected one." % (s, t))
            # exit(-1)
        graph[s].append((t, w))
        graph[t].append((s, w))
    return graph

def write_output(n, edges, graph, output_filename):
    kahip_vertices = n
    kahip_edges = len(edges)
    kahip_f = 1

    with open(output_filename, 'w') as f:
        f.write('%d %d %d\n' % (kahip_vertices, kahip_edges, kahip_f))
        for s in range(n):
            # ddsg is 0-based, kahip is 1-based
            f.write(' '.join('%d %d' % (t + 1, w) for (t, w) in graph[s]))
            f.write('\n')

def main():
    input_filename, output_filename = parse_args()
    n, m, edges = parse_input(input_filename)
    graph = generate_graph(edges)
    write_output(n, edges, graph, output_filename)


if __name__ == '__main__':
    main()
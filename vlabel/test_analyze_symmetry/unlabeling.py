
import sys
import glob

def unlabeling(input_file_name, output_file_name):
    input_file = open(input_file_name, 'r')
    output_file = open(output_file_name, 'w')

    lines = input_file.readlines()

    for line in lines:
        line = line.strip()
        words = line.split()
        if words[0] == 't':
            output_file.write(line + '\n')
        elif words[0] == 'v':
            output_file.write('v {} {} {}\n'.format(words[1], 0, words[3]))
        elif words[0] == 'e':
            output_file.write(line + '\n')

    input_file.close()
    output_file.close()

if __name__ == '__main__':
    
    
    

    input_dir = sys.argv[1]
    output_dir = sys.argv[2]

    
    query_graph_path_list = glob.glob('{0}/*.graph'.format(input_dir))

    for query in query_graph_path_list:
        query_graph_name = query.split('/')[-1].split('.')[0]
        unlabeling(query, '{0}/{1}.graph'.format(output_dir, query_graph_name))
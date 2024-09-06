

import sys
import os
import glob
from subprocess import Popen, PIPE


def generate_args(binary, *params):
    arguments = [binary]
    arguments.extend(list(params))
    return arguments


def execute_binary(args):
    process = Popen(' '.join(args), shell=True, stdout=PIPE, stderr=PIPE)
    (std_output, std_error) = process.communicate()
    process.wait()
    rc = process.returncode

    return rc, std_output, std_error


def check_correctness_of_symmetry_breaking(binary_path, data_graph_path, query_folder_path, parameter):
    output_file = 'symmetry_breaking.res'
    with open(output_file, 'w') as f:

        filter = parameter[0]
        order = parameter[1]
        engine = parameter[2]

        
        query_graph_path_list = glob.glob('{0}/*.txt'.format(query_folder_path))

        for query_graph_path in query_graph_path_list:
            query_graph_name = os.path.splitext(os.path.basename(query_graph_path))[0]
            print(query_graph_name)

        
        for query_graph_path in query_graph_path_list:
            query_graph_name = os.path.splitext(os.path.basename(query_graph_path))[0]
            print("start:", query_graph_name)

            
            execution_args = generate_args(binary_path, '-d', query_graph_path, '-q', query_graph_path, '-filter',
                                        filter, '-order', order, '-engine', engine)

            (rc, std_output, std_error) = execute_binary(execution_args)

            if rc == 0:
                automorphism_num = 0
                std_output_list = std_output.split('\n')
                for line in std_output_list:
                    if '
                        automorphism_num = int(line.split(':')[1].strip())
                        
                        break
            else:
                print'find the automorphism number: {0} engine {1} error.'.format(engine, query_graph_name)
                print(std_output)
                print(std_error)
                exit(-1)

            
            execution_args = generate_args(binary_path, '-d', data_graph_path, '-q', query_graph_path, '-filter',
                                            filter, '-order', order, '-engine', engine)    
            (rc, std_output, std_error) = execute_binary(execution_args)

            if rc == 0:
                wo_symmetry_num = 0
                std_output_list = std_output.split('\n')
                for line in std_output_list:
                    if '
                        wo_symmetry_num = int(line.split(':')[1].strip())
                        
                        break
            else:
                print'without symmetry breaking: {0} engine {1} error.'.format(engine, query_graph_name)
                print(std_output)
                print(std_error)
                exit(-1)

            
            execution_args = generate_args(binary_path, '-d', data_graph_path, '-q', query_graph_path, '-filter',
                                            filter, '-order', order, '-engine', engine, "-symmetry", "1");    
            (rc, std_output, std_error) = execute_binary(execution_args)

            if rc == 0:
                with_symmetry_num = 0
                std_output_list = std_output.split('\n')
                for line in std_output_list:
                    if '
                        with_symmetry_num = int(line.split(':')[1].strip())
                        
                        break
            else:
                print'with symmetry breaking: {0} engine {1} error.'.format(engine, query_graph_name)
                print(std_output)
                print(std_error)
                exit(-1)

            f.write('{0}, {1}, {2}, {3}\n'.format(query_graph_name, automorphism_num, wo_symmetry_num, with_symmetry_num))

            if automorphism_num * with_symmetry_num != wo_symmetry_num:
                    
                print 'Symmetry breaking {0} engine {1} is wrong.'.format(engine, query_graph_name)
                print 'Automorphism number: {0}, Without symmetry breaking: {1}, With symmetry breaking: {2}'.format(automorphism_num, wo_symmetry_num, with_symmetry_num)

                print(std_output)
                print(std_error)
                exit(-1)
            
        print "Symmetry breaking {0} engine pass the correctness check.".format(engine)


def check_correctness_of_automorphism(binary_path, query_folder_path, parameter):
    output_file = 'self_correctness.res'
    with open(output_file, 'w') as f:
        
        filter = parameter[0]
        order = parameter[1]
        engine = parameter[2]

        
        query_graph_path_list = glob.glob('{0}/*'.format(query_folder_path))

        for query_graph_path in query_graph_path_list:
            query_graph_name = os.path.splitext(os.path.basename(query_graph_path))[0]
            print(query_graph_name)

        
        for query_graph_path in query_graph_path_list:
            query_graph_name = os.path.splitext(os.path.basename(query_graph_path))[0]
            print("start:", query_graph_name)

            
            execution_args = generate_args(binary_path, '-d', query_graph_path, '-q', query_graph_path, '-filter',
                                            filter, '-order', order, '-engine', engine)

            (rc, std_output, std_error) = execute_binary(execution_args)

            if rc == 0:
                automorphism_num = 0
                std_output_list = std_output.split('\n')
                for line in std_output_list:
                    if '
                        automorphism_num = int(line.split(':')[1].strip())
                        
                        break
            else:
                print'find the automorphism number: {0} engine {1} error.'.format(engine, query_graph_name)
                print(std_output)
                print(std_error)
                exit(-1)

            
            execution_args = generate_args(binary_path, '-d', query_graph_path, '-q', query_graph_path, '-filter',
                                            filter, '-order', order, '-engine', engine, "-symmetry", "1");    
            (rc, std_output, std_error) = execute_binary(execution_args)

            if rc == 0:
                with_symmetry_num = 0
                std_output_list = std_output.split('\n')
                for line in std_output_list:
                    if '
                        with_symmetry_num = int(line.split(':')[1].strip())
                        
                        break
            else:
                print'with symmetry breaking: {0} engine {1} error.'.format(engine, query_graph_name)
                print(std_output)
                print(std_error)
                exit(-1)

            f.write('{0}, {1}, {2}\n'.format(query_graph_name, automorphism_num, with_symmetry_num))

            if with_symmetry_num != 1:
                    
                print 'Symmetry breaking {0} engine {1} is wrong.'.format(engine, query_graph_name)
                print 'Automorphism number: {0}, With symmetry breaking: {1}'.format(automorphism_num, with_symmetry_num)

                print(std_output)
                print(std_error)
                exit(-1)

        print "Symmetry breaking {0} engine pass the correctness check.".format(engine)


if __name__ == '__main__':
    input_binary_path = sys.argv[1]
    if not os.path.isfile(input_binary_path):
        print 'The binary {0} does not exist.'.format(input_binary_path)
        exit(-1)

    method = ['DPiso', 'RI', 'LFTJ']

    
    input_data_graph_path = '/root/subgraph/code/backtracking/test_analyze_symmetry/unlabelled_data_graph/HPRD.graph'
    input_query_graph_folder_path = '/root/subgraph/code/backtracking/test_analyze_symmetry/toy'

    check_correctness_of_symmetry_breaking(input_binary_path, input_data_graph_path, input_query_graph_folder_path, method)

    
    input_query_graph_folder_path = '/root/subgraph/code/backtracking/test_analyze_symmetry/toy'
    check_correctness_of_automorphism(input_binary_path, input_query_graph_folder_path, method)

    
    input_data_graph_path = '/root/subgraph/code/backtracking/test/data_graph/HPRD.graph'
    input_query_graph_folder_path = '/root/subgraph/code/backtracking/test/query_graph'
    check_correctness_of_symmetry_breaking(input_binary_path, input_data_graph_path, input_query_graph_folder_path, method)
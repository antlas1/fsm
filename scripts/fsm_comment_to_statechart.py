from enum import Enum
import sys
import os
import codecs
import fileinput
import argparse

class State(Enum):    
    NO_DIA = 1,
    DIA_HEAD = 2,
    DIA_STATE = 3

STR_DIA_HEADER = '//@statechart'
STR_DIA_FOOTER = '//@endchart'
STR_DIA_STR = '//@'
STR_ACT_STR = '///'
STR_ACTION_STARTER = ' / '
STR_ACTION_SPLITTER = '\\n*'

def log(inp):
    #TODO: добавить глобальный переключатель
    #print(inp)
    pass

#комментарий описывает переход из одного состояния в другое
def is_state_trans_line(line):
    global STR_DIA_STR
    return (STR_DIA_STR in line) and ( ('-->' in line) or ('->' in line) )

def get_plant_string(line,delim):
    try:
        proc_line = line.strip()
        index = proc_line.index(delim)
        return proc_line[index+len(delim):]
    except ValueError:
        print("Not found dia string!")
        sys.exit(-1)
        
def get_dia_name(line):
    global STR_DIA_HEADER
    try:
        proc_line = line.strip()
        index = proc_line.index(STR_DIA_HEADER)
        name = proc_line[index+len(STR_DIA_HEADER):]
        return name.strip()
    except ValueError:
        print("Not found dia header!")
        sys.exit(-1)

def process_file(input_fname):
    #print(input_fname)
    dia_name = ''
    notes = []
    state_desc = []
    curr_state_actions = []
    state = State.NO_DIA
    have_result = False
    result_puml = {}
    result_doxy = {}
    with codecs.open(input_fname,'r','utf-8', errors='ignore') as f_in:
        for line in f_in:
            if state == State.NO_DIA:
                if STR_DIA_HEADER in line:
                    dia_name = get_dia_name(line)
                    log('Start dia `{}`'.format(dia_name))
                    state = State.DIA_HEAD
            elif state == State.DIA_HEAD:
                if is_state_trans_line(line):
                    log('Start first state')
                    curr_state_line = get_plant_string(line,STR_DIA_STR)
                    state_desc.append(curr_state_line)
                    state = State.DIA_STATE
                elif STR_DIA_STR in line:
                    notes.append(get_plant_string(line,STR_DIA_STR))
            elif state == State.DIA_STATE:
                if STR_DIA_FOOTER in line:
                    if len(curr_state_actions) > 0:
                        curr_state_line = state_desc.pop(-1)
                        acts = STR_ACTION_SPLITTER.join(curr_state_actions)
                        curr_state_line = curr_state_line + STR_ACTION_STARTER + STR_ACTION_SPLITTER + acts
                        curr_state_actions = []
                        state_desc.append(curr_state_line)
                    curr_state_line = get_plant_string(line,STR_DIA_STR)
                    log('End dia')
                    
                    if dia_name != '':
                        have_result = True
                        dia_code = dia_name.split()[0]
                        result_doxy[dia_code] = """/*! \\page {}
\startuml
{}

legend
  {}
endlegend
\enduml
*/
                        """.format(dia_name,'\n'.join(state_desc), '\n  '.join(notes))

                        result_puml[dia_code] = """@startuml
{}


legend
  {}
endlegend
@enduml

                        """.format('\n'.join(state_desc), '\n  '.join(notes))
                    print('Create digram {}'.format(dia_name))
                    state = State.NO_DIA
                    dia_name = ''
                    notes = []
                    state_desc = []
                    curr_state_line = ''
                    curr_state_actions = []
                elif STR_ACT_STR in line:
                    curr_state_actions.append(get_plant_string(line,STR_ACT_STR))
                    log('New act')
                elif STR_DIA_STR in line:
                    if len(curr_state_actions) > 0:
                        curr_state_line = state_desc.pop(-1)
                        acts = STR_ACTION_SPLITTER.join(curr_state_actions)
                        curr_state_line = curr_state_line + STR_ACTION_STARTER + STR_ACTION_SPLITTER + acts
                        curr_state_actions = []
                        state_desc.append(curr_state_line)
                    state_desc.append(get_plant_string(line,STR_DIA_STR))
                    log('New transition')

    if have_result:
        #print('Result:\n{}'.format(result))
        for key, value in result_doxy.items():
            dest_doxy = os.path.join(os.path.dirname(input_fname),key+'.dox')
            with codecs.open(dest_doxy,'w','utf-8') as f_out:
                f_out.write(value)
                
        for key, value in result_puml.items():
            dest_doxy = os.path.join(os.path.dirname(input_fname),key+'.puml')
            with codecs.open(dest_doxy,'w','utf-8') as f_out:
                f_out.write(value)
                
        #class_name = os.path.splitext(os.path.basename(input_fname))[0]
        #dest_puml = os.path.join(os.path.dirname(input_fname),class_name+'.puml')
        #with codecs.open(dest_puml,'w','utf-8') as f_out:
        #    f_out.write(result_puml)
        
 
if __name__ == "__main__": 
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter, description='Create diagrams for FSM embedded in C++ code')
    parser.add_argument("input",type=str, help="Input directory with source files")
    args = parser.parse_args()
    
    print('Processing directory `{}`'.format(args.input))
    for subdir, dirs, files in os.walk(args.input):
        for file in files:
            filepath = os.path.join(subdir, file)
            if filepath.endswith(".cpp"):
                print('Process {}'.format(filepath))
                process_file(filepath)
    print('Done')

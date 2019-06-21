#!/usr/bin/python3
import json
import sys
import os 

output_filename="jsonParser.hpp"
def recParse_structure(toParse,nestLevel=0):
    global output_filename
    struct_name=os.path.basename(output_filename)
    struct_name=os.path.splitext(struct_name)[0]+"_format"
    add_end=False
    if nestLevel == 0:
        print("typedef struct "+struct_name+"_t{")
        add_end=True
        nestLevel+=1
    for p in toParse:
        if isinstance(toParse[p],dict):
            print("\t"*nestLevel+"struct "+p+"_t{")
            recParse_structure(toParse[p],nestLevel+1)
            print("\t"*nestLevel+"}"+p+";")
        if isinstance(toParse[p],int):
            print("\t"*nestLevel+"int "+p+";")
        if isinstance(toParse[p],float):
            print("\t"*nestLevel+"float "+p+";")
        if isinstance(toParse[p],str):
            print("\t"*nestLevel+"std::string "+p+";")
    if add_end:
        print("} "+struct_name+";")

def recParse_function(toParse,nestLevel=0,previousLevelVar="O",variablePath=[]):
    global output_filename
    struct_name=os.path.basename(output_filename)
    funct_name="parse_"+os.path.basename(output_filename)
    struct_name=os.path.splitext(struct_name)[0]+"_format"
    add_end=False
    if nestLevel == 0:
        print(struct_name+" parse_json_file(const char *filename){")
        nestLevel+=1
        print("\t"*nestLevel+"std::ifstream confFileStream(filename);")
        print("\t"*nestLevel+"if(!confFileStream.good()){")
        print("\t"*nestLevel+"\tstd::cout<<\"Could not open file:\"<< filename<<\""+r"\n"+"\";")
        print("\t"*nestLevel+"}")
        print("\t"*nestLevel+"std::stringstream buffer;")
        print("\t"*nestLevel+"buffer<< confFileStream.rdbuf();")
        print("\t"*nestLevel+struct_name+" param_out;")
        print("\t"*nestLevel+"Expected<json::Value> param = json::parse(buffer.str());")
        print("\t"*nestLevel+"if(param){")
        nestLevel+=1
        print("\t"*nestLevel+"json::Object* O = param->getAsObject();")
        add_end=True
        nestLevel+=1
    for p in toParse:
        if isinstance(toParse[p],dict):
            print("\t"*nestLevel+"if (json::Object* o_"+p+" = "+previousLevelVar +"->getObject(\""+p+"\")){")
            recParse_function(toParse[p],nestLevel+1,"o_"+p,variablePath+[p])
            print("\t"*nestLevel+"}")
        if isinstance(toParse[p],int):
            print("\t"*nestLevel+"if(Optional<int64_t> "+variablePath[-1]+"_"+p+" = "+previousLevelVar+"->getInteger(\""+p+"\")){")
            print("\t"*nestLevel+"\tif("+variablePath[-1]+"_"+p+".hasValue()){ ")
            print("\t"*nestLevel+"\t\tparam_out",end="")
            for i in variablePath:
                print("."+i,end="")
            print("."+p+" = "+ variablePath[-1]+"_"+p+".getValue();")
            print("\t"*nestLevel+"\t}else{ ")
            print("\t"*nestLevel+"\t\tparam_out",end="")
            for i in variablePath:
                print("."+i,end="")
            print("."+p+" = "+ str(toParse[p])+";")
            print("\t"*nestLevel+"\t}")
            print("\t"*nestLevel+"}")
        if isinstance(toParse[p],float):
            print("\t"*nestLevel+"float "+p+";")
        if isinstance(toParse[p],str):
            print("\t"*nestLevel+"std::string "+p+";")
    if add_end:
        print("\t"*nestLevel+"}")
        print("\t"*nestLevel+"else{")
        print("\t"*nestLevel+"\tif(auto Err =handleErrors(param.takeError(),[](const json::ParseError &PE){")
        print("\t"*nestLevel+"\t\terrs()<< \"Couldn't parse the parameter file"+r"\n"+"\";")
        print("\t"*nestLevel+"\t\t}) )")
        print("\t"*nestLevel+"\terrs()<<\"Unexpected Error"+r"\n"+"\";")
        print("\t"*nestLevel+"}")

                
        print("return param_out;")
        print("}")


def main():
    global output_filename
    if len(sys.argv) < 2:
        print("You need to provide an input json file")
        return 
    json_filename = sys.argv[1]
    if len(sys.argv) == 3:
        print("Redirecting output to "+sys.argv[2])
        output_filename = sys.argv[2]
        sys.stdout = open(output_filename,'w')
    if not os.path.exists(json_filename):
        print("I couldn't find the input json file"+json_filename)
        return
    with open(json_filename) as json_file:
        print("#ifndef "+output_filename.upper().replace('.','_'))
        print("#define "+output_filename.upper().replace('.','_'))
        print("")
        print("#include \"llvm/Support/JSON.h\" //to parse raw JSON")
        print("#include <fstream> //to write/read files")
        print("#include <iostream> //to use cout")
        print("#include <sstream> //to use stringstream")
        print("")
        print("using namespace llvm;")
        print("")
        data = json.load(json_file)
        recParse_structure(data)
        print("")
        recParse_function(data)
        print("")
        print("#endif")


if __name__ == "__main__":
    main()

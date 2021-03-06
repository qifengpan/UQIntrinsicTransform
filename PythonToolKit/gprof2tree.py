#!/usr/bin/python

import string
import sys

def find_call_tree(fp):
    """Skips the gprof output up to the point where the call tree
       information starts"""
    line = fp.readline()
    while line and (line[0:7] != "index %" and line[0:8] != "index  %"):
        line = fp.readline()
    if line[0:8] == "index  %":
        line = fp.readline()
        line = fp.readline()

def parse_call_tree(fp,parents,children,times):
    """Parse the call tree information from the gprof output file and store
       it in three dictionaries:
       - parents:  contains the routines that call a subroutine
       - children: contains the routines called by a subroutine
       - times:    contains cpu-time,call-counts tuples for every subroutine,
                   child tuple."""
    parse_parents = 1
    parse_children = 2
    mode = parse_parents
    slist = []
    line = fp.readline()
    while line:
        if len(line) <= 1:
#
#           empty line: nothing to do but get next line
#
            pass
#
        elif line[0] == "\f":
#
#           end of table found, we are done
#
            return
#
        elif line[0:11] == " This table":
#
#           end of table found, we are done
#
            return
#
        elif string.find(line,"<spontaneous>") > -1:
#
#           found "spontaneous" parent, add it to the current subroutine list
#
            slist.append("<spontaneous>")
            print('slist after appending is: ',slist)
            
#
        elif line[0] == " ":
#
#           Found a parent or a child, the line may come in two different
#           formats:
#
#           1. listing the fields: self, children, called, name
#           2. listing the fields: called, name (if recursive)
#
#           E.g.:
#
#           0.00        0.00       1/1           .main [45]
#                                 49             .fac1 <cycle 1> [3]
#
            tlist = string.split(line,None,3)
            print('tlist after appending is: ',tlist)
            if string.find(tlist[0],".") == -1:
#
#              this is a type 2 line, so resplit it accordingly
#
               tlist = string.split(line,None,1)
               print('tlist for type 2: ', tlist)
               tcput = 0.0
               tcalled = tlist[0]
               tname = tlist[1]
            else:
#
#              this is a type 1 line
#
               tcput = tlist[1]
               tcalled = tlist[2]
               tname = tlist[3]
#
#           Now strip off the index number at the end of the name
#
            i = string.rfind(tname," [")
            tname = tname[:i]
#
#           Now strip the potential "<cycle xxx>" off
#
            i = string.rfind(tname," <cycle ")
            if i > -1:
               tname = tname[:i]
#
#           Now the name is the pure function/subroutine name
#
            slist.append(tname)
            if mode == parse_children:
               times[(subrname,tname)] = (tcput,tcalled)
#
        elif line[0] == "[":
#
#           Now we have found the function/subroutine itself
#           this line comes in the format:
#
#             index %time self children called name
#
#           E.g.:
#
#             [1]     35.3    0.00        1.18       1         .dl_poly [1]
#
            tlist = string.split(line,None,5)
            tcput = tlist[3]
            tcalled = tlist[4]
            tname = tlist[5]
#
#           Now strip off the index number at the end of the name
#
            i = string.rfind(tname," [")
            tname = tname[:i]
#
#           Now strip the potential "<cycle xxx>" off
#
            i = string.rfind(tname," <cycle ")
            if i > -1:
               tname = tname[:i]
#
            subrname = tname
            parents[subrname] = slist
            slist = []
            mode = parse_children
#
        elif line[0] == "-":
#
#           found the line that terminates the children section
#           so switch mode to prepare for another parents section
#
            children[subrname] = slist
            slist = []
            mode = parse_parents
#
        else:
#
#           I am not sure about this...
#
            print("Do not know how to interpret the following line:")
            print(line)
            print("Giving up...")
            sys.exit(2)
#
        line = fp.readline()

def sort_call_tree(treedata):
    """Given some call tree this routine will sort the related items
       (usually parents or children). This way a particular order can
       be garanteed irrespective of the amount of CPU time a subroutine
       consumes."""
    for name in treedata:
        treedata[name].sort()

def exclude_call_tree(filenames,treedata):
    """Given a number of files this routine reads the files and deletes
       any names listed there from the treedata."""
    for filename in filenames:
        fp = open(filename,"r")
        line = fp.readline()
        while line:
            if line[0] != "#":
                slist = string.split(line)
                if len(slist) > 0:
                    key = slist[0]
                    if treedata.has_key(key):
                        del treedata[key]
            line = fp.readline()
        fp.close()

def stopat_call_tree(filenames,treedata):
    """Given a number of files this routine reads the files and deletes
       any children or parents of the routines listed there from the
       treedata."""
    for filename in filenames:
        fp = open(filename,"r")
        line = fp.readline()
        while line:
            if line[0] != "#":
                slist = string.split(line)
                if len(slist) > 0:
                    key = slist[0]
                    if treedata.has_key(key):
                        treedata[key] = []
            line = fp.readline()
        fp.close()

def write_call_tree(fp,indent,subrname,treedata,stack,once):
    """Given a subroutine name this routine recursively prints out its
       call tree. Before printing the subroutine name it is checked that the
       name actually appears in the treedata dictionary. If it does not exist
       then that probably means the routine got deleted.
       Finally there is the option of printing the subtree related to a
       subroutine only once. This is arranged in a way similar to the
       'stopat' approach by deleting all the children."""
    if treedata.has_key(subrname):
        if stack.count(subrname) == 2:
            fp.write(indent+subrname+" (recursive)\n")
        else:
            fp.write(indent+subrname+"\n")
            slist = treedata[subrname]
            for name in slist:
                if stack.count(name) < 2:
                    stack.append(name)
                    write_call_tree(fp,indent+"  ",name,treedata,stack,once)
                    name = stack.pop()
        if once:
            treedata[subrname] = []

def write_call_tree_level(fp,indent,subrname,treedata,stack,level,once):
    """Given a subroutine name this routine recursively prints out its
       call tree. Before printing the subroutine name it is checked that the
       name actually appears in the treedata dictionary. If it does not exist
       then that probably means the routine got deleted.
       Finally there is the option of printing the subtree related to a
       subroutine only once. This is arranged in a way similar to the
       'stopat' approach by deleting all the children."""
    if treedata.has_key(subrname):
        if stack.count(subrname) == 2:
            fp.write("%3d %s\n" % (level,indent+subrname+" (recursive)"))
        else:
            fp.write("%3d %s\n" % (level,indent+subrname))
            slist = treedata[subrname]
            for name in slist:
                if stack.count(name) < 2:
                    stack.append(name)
                    write_call_tree_level(fp,indent+"  ",name,treedata,stack,level+1,once)
                    name = stack.pop()
        if once:
            treedata[subrname] = []


def search_to_call_tree(fp,subrname,tolist,treedata,stack,minlevel,curlevel):
    """Given a subroutine name this routine recursively searches for parents
       or children that a included in the tolist. If a routine on the tolist
       is found then the call stack is printed."""
    if treedata.has_key(subrname):
        stack.append(subrname)
        if tolist.count(subrname) >= 1:
            print_to_call_tree(fp,stack,minlevel)
            minlevel = curlevel+1
        if stack.count(subrname) < 2:
            slist = treedata[subrname]
            for name in slist:
                minlevel = search_to_call_tree(fp,name,tolist,treedata,stack,minlevel,curlevel+1)
            if curlevel < minlevel:
                minlevel = curlevel
        name = stack.pop()
    return minlevel

def print_to_call_tree(fp,stack,minlevel):
    """Print the call stack from minlevel down"""
    indent = ""
    for i in range(0,len(stack)):
        if i >= minlevel:
            if stack.count(stack[i]) == 2:
                fp.write(indent+stack[i]+" (recursive)\n")
            else:
                fp.write(indent+stack[i]+"\n")
        indent = indent + "  "

def print_subroutine_name(fp,name):
    """Print what subroutine we are looking for."""
    fp.write("Printing call tree for function/subroutine: "+name+"\n")

def check_subroutine_name(fp,name,dict):
    """Check that the subroutine we are looking for actually exists.
       If not report back to the user and exit."""
    if dict.has_key(name):
       pass
    else:
       fp.write("Function/subroutine: "+name+" not found.\n")
       fp.write("Please check your gprof output file.\n")
       sys.exit(3)

def version_gprof2tree(fp):
    """Print out the version information."""
    revision = "$Revision: 1.4 $"
    date     = "$Date: 2006/03/16 15:36:03 $"
    rlist    = string.split(revision)
    dlist    = string.split(date)
    version  = "gprof2tree version "+rlist[1]+" of "+dlist[1]+"\n"
    fp.write(version)

def usage_gprof2tree(fp):
    """Print out the usage information."""
    fp.write("""
    Usage:

       gprof2tree [--parents] [--children] [--[no]sort] [--once|--always]
                  [--exclude "<file1>[:<file2>[:<file3>]]"] [--help]
                  [--stopat "<file4>[:<file5>[:<file6>]]"] [--version]
                  [--to "<routine name1>[ <routine name2>[ <routine name3>]]"]
                  <subroutine name>

    This script reads the call tree data from a gprof output and prints out
    the call tree of the routine <subroutine name>. The following options
    can be used

    --help        Print this information.

    --parents     Print the call moving upwards. I.e. print the routines that
                  call <subroutine name>.

    --children    Print the call moving downwards. I.e. print the routines that
                  routine <subroutine name> calls. This is the default.

    --sort        Alphabetically sort the routine names in the call tree.
                  This is useful if the call trees of slighty different
                  runs have to be compared as it reduces the differences 
                  introduced by changes of CPU usage.

    --nosort      Print the routines in the call tree in the same order as
                  found in the gprof call tree information. Usually this means
                  that the routines with the highest CPU usage come first.
                  This is the default.

    --once        Print the subtree for every subroutine only once, i.e.
                  the first time it is encountered. In subsequent encounters
                  only the call is printed. This helps to greatly reduce the
                  length of the printed call trees and is therefore the
                  default.

    --always      Always print the tree of every subroutine encountered
                  limited only by the 'stopat' and 'exclude' options in effect.

    --exclude     Removes all the routines that are named in the specified 
                  files from the call tree. This is useful in larger codes
                  where there can a lot of routines that are of no particular
                  interest.

    --stopat      Removes the call tree information of the routines that
                  are named in the specified files. The routine names themselves
                  are still printed in the call trees but not the routines
                  they call. This is particularly useful in larger codes which
                  may have deep call trees. In which one may want to know where
                  a routine is called but not how it goes about doing its job.

    --to          Prints only the paths throught the code that lead from 
                  the desired subroutine to the routines named in the to-list.
                  All other paths are suppressed.

    --level       Print the depth level of a routine in the call tree on 
                  every line.

    --version     Print the version number of gprof2tree.

    Copyright 2004-2006, Huub van Dam, CCLRC Daresbury Laboratory
    \n""")
    sys.exit(1)


parents = {}
parents["<spontaneous>"] = []
children = {}
children["<spontaneous>"] = []
times = {}
stack = []
tolist = []
true = (0 == 0)
false = not true

narg = len(sys.argv)
iarg = 1
osort = false
oparent = false
ochild = true
oexclude = false
ostopat = false
oto = false
olevel = false
oonce = true
subrname = " "
minlevel = 0
while iarg < narg:
    if sys.argv[iarg] == "--parents":
        oparent = true
        ochild = false
        iarg = iarg + 1
    elif sys.argv[iarg] == "--children":
        ochild = true
        oparent = false
        iarg = iarg + 1
    elif sys.argv[iarg] == "--sort":
        osort = true
        iarg = iarg + 1
    elif sys.argv[iarg] == "--nosort":
        osort = false
        iarg = iarg + 1
    elif sys.argv[iarg] == "--once":
        oonce = true
        iarg = iarg + 1
    elif sys.argv[iarg] == "--always":
        oonce = false
        iarg = iarg + 1
    elif sys.argv[iarg] == "--exclude":
        oexclude = true
        excludefiles = string.split(sys.argv[iarg+1],":")
        iarg = iarg + 2
    elif sys.argv[iarg] == "--stopat":
        ostopat = true
        stopatfiles = string.split(sys.argv[iarg+1],":")
        iarg = iarg + 2
    elif sys.argv[iarg] == "--to":
        oto = true
        tolist = string.split(sys.argv[iarg+1]," ")
        iarg = iarg + 2
    elif sys.argv[iarg] == "--level":
        olevel = true
        iarg = iarg + 1
    elif sys.argv[iarg] == "--version":
        version_gprof2tree(sys.stdout)
        iarg = iarg + 1
    elif sys.argv[iarg] == "--help":
        usage_gprof2tree(sys.stdout)
        iarg = iarg + 1
    else:
        subrname = str(sys.argv[iarg])
        iarg = iarg + 1
if subrname == " ":
    usage_gprof2tree(sys.stdout)

print_subroutine_name(sys.stderr,subrname)

find_call_tree(sys.stdin)
print('parents before parsing: ',parents )
print('children before parsing: ',children)
parse_call_tree(sys.stdin,parents,children,times)
print('parents after parsing: ',parents)
print('children after parsing: ',children)
check_subroutine_name(sys.stderr,subrname,children)
if ochild:
    if oexclude:
        exclude_call_tree(excludefiles,children)
    if ostopat:
        stopat_call_tree(stopatfiles,children)
    if osort:
        sort_call_tree(children)
    if oto:
        minlevel = search_to_call_tree(sys.stdout,subrname,tolist,children,stack,minlevel,0)
    else:
        if olevel:
            write_call_tree_level(sys.stdout,"",subrname,children,stack,1,oonce)
        else:
            write_call_tree(sys.stdout,"",subrname,children,stack,oonce)
if oparent:
    if oexclude:
        exclude_call_tree(excludefiles,parents)
    if ostopat:
        stopat_call_tree(stopatfiles,parents)
    if osort:
        sort_call_tree(parents)
    if oto:
        minlevel = search_to_call_tree(sys.stdout,subrname,tolist,parents,stack,minlevel,0)
    else:
        if olevel:
            write_call_tree_level(sys.stdout,"",subrname,parents,stack,1,oonce)
        else:
            write_call_tree(sys.stdout,"",subrname,parents,stack,oonce)



Memory Dumps @ The unmanaged domain

The focus of this article is to discuss an atonomus method of generating dump files w/o the need of any development tool installed.</br>
Is starts by giving a high level explenation of what dump files are and what they are used for, then, it present few of the most common development tools used to generate dump files and discuss windows exception model, finally, a way of generating dump files w/o the need of any development tool is presented.


The sample project consists of a single CPP file ( DumpFiles.cpp ) where the logic is implemented, To Integrate with other projects, simply cut & paste into your solution all that is under the 'Dumps" namespace

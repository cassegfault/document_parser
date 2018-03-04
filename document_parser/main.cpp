//
//  main.cpp
//  document_parser
//
//  Created by Chris Pauley on 3/1/17.
//  Copyright © 2017 Infegy. All rights reserved.
//

#include <iostream>
#include <fstream>
#include "document_parser.h"

using namespace std;

int main(int argc, const char * argv[]) {
    vector<string> file_list = {
        "test1.html",
        "test2.html",
        "test3.html",
        "test4.html",
        "test5.html",
        "test6.html",
        "test7.html",
        "test8.html",
        "test9.html",
        "test10.html",
        "test11.html",
    };
    
    for(auto filename : file_list){
        
        // Read the file in
        string in_filename = "tests/";
        in_filename += filename;
        ifstream t(in_filename.c_str());
        string str;
        
        t.seekg(0, ios::end);
        str.reserve(t.tellg());
        t.seekg(0, ios::beg);
        
        str.assign((istreambuf_iterator<char>(t)),istreambuf_iterator<char>());
        
        
        
        ofstream out_file;
        string out_filename = "out/";
        out_filename += filename;
        out_file.open(out_filename.c_str());
        
        document_parser doc(str);
        string out_html = doc.clean_html();
        out_file.write(out_html.c_str(), out_html.size());
        
        out_file.close();
        
        cout << doc.title << endl;
        cout << "\tWritten By " << doc.author << " on " << doc.date << endl;
    }
    
    return 0;
}

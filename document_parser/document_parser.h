//
//  document_parser.hpp
//  document_parser
//
//  Created by Chris Pauley on 3/12/17.
//  Copyright © 2017 Infegy. All rights reserved.
//

#ifndef document_parser_h
#define document_parser_h
#include <unordered_set>
#include <unordered_map>
#include <infegy/html_element.h>
#include <infegy/utilities.h>

using namespace std;
class document_parser {
public:
    document_parser(string document_body);
    
    html_element root_element;
    
    string title;
    string author;
    string date;
    
    pair<string, int> score_author(html_element * el);
    pair<string, int> score_title(html_element * el);
    pair<string, int> score_date(html_element * el);
    int score_element(html_element * el);
    
    unordered_map<html_element *, int> score_map;
    unordered_map<string, int> class_counts;
    vector<html_element *> body_elements;
    
    string clean_html();
    vector<string> list_items();
};

// For list identification
const unordered_set<string> not_a_list = { "nav", "footer" };
const unordered_set<string> author_markers = { "byline", "author", "writtenby" };
const unordered_set<string> date_markers = { "byline", "date", "time"};
const unordered_set<string> meta_date_markers = { "date", "time" };
const unordered_set<string> meta_author_markers = { "author", "site_title" };
const unordered_set<string> bad_attributes = { "campaign" };
const unordered_set<string> positive_classes = { "article","body","content","entry","hentry","h-entry","main","page","pagination","post","text","blog","story" };
const unordered_set<string> negative_classes = { "hidden","hid","banner","combx","comment","com-","contact","foot","footer","footnote","masthead","media","meta","modal","outbrain","promo","related","scroll","share","shoutbox","sidebar","skyscraper","sponsor","shopping","tags","tool","widget","vid","ad-","ad_", "gal-", "gallery", "tool", "story-frag", "supplement" };

#endif /* document_parser_h */

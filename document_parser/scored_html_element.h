//
//  scored_html_element.hpp
//  document_parser
//
//  Created by Chris Pauley on 3/12/17.
//  Copyright © 2017 Infegy. All rights reserved.
//

#ifndef scored_html_element_h
#define scored_html_element_h

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <infegy/utilities.h>
#include <infegy/html_element.h>




using namespace std;

class scored_html_element{
    friend class html_element;
public:
    scored_html_element(string to_parse);
    scored_html_element(html_element base);
    
    int score;
    pair<string, int> title_score();
    pair<string, int> author_score();
    pair<string, int> date_score();
    
    vector<scored_html_element> children;
    
private:
    void _build_score();
};


unordered_map<string, int> tag_scores = {
        { "div", 0 },
        { "p", 5 },
        
        { "pre", 3 },
        { "td", 3 },
        { "blockquote", 3 },
        
        { "address" , -3 },
        { "ol" , -3 },
        { "ul" , -3 },
        { "dl" , -3 },
        { "dd" , -3 },
        { "dt" , -3 },
        { "li" , -3 },
        { "form" , -3 },
        
        { "h1", -5 },
        { "h2", -5 },
        { "h3", -5 },
        { "h4", -5 },
        { "h5", -5 },
        { "h6", -5 },
        { "th", -5 }
    };
unordered_map<string, int> title_tag_scores = {
        { "h1", 5 },
        { "h2", 3 },
        { "h3", 1 },
    };

const unordered_set<string> positive_classes = { "article","body","content","entry","hentry","h-entry","main","page","pagination","post","text","blog","story" };

const unordered_set<string> negative_classes = { "hidden","hid","banner","combx","comment","com-","contact","foot","footer","footnote","masthead","media","meta","modal","outbrain","promo","related","scroll","share","shoutbox","sidebar","skyscraper","sponsor","shopping","tags","tool","widget","vid","ad-","ad_", "gal-", "gallery", "tool", "story-frag", "supplement" };

const unordered_set<string> bad_attributes = { "campaign" };

const unordered_set<string> author_markers = { "byline", "author", "writtenby" };

const unordered_set<string> meta_author_markers = { "author", "site_title" };

const unordered_set<string> meta_date_markers = { "date", "time" };

const unordered_set<string> date_markers = { "byline", "date", "time"};


#endif /* scored_html_element_h */

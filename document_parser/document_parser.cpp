//
//  document_parser.cpp
//  document_parser
//
//  Created by Chris Pauley on 3/12/17.
//  Copyright © 2017 Infegy. All rights reserved.
//

#include "document_parser.h"
#include <iostream>

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

pair<string, int> document_parser::score_author(html_element * el){
    string el_class = str_as_lower(el->attribute("class") ),
    el_id = str_as_lower(el->get_id()),
    author;
    size_t body_size = 0;
    for(auto & child : el->children){
        if(child.tag.size() == 0){
            body_size += child.text().size();
        }
    }
    int score = 0;
    for(auto marker : author_markers){
        if(str_contains(el_class, marker))
            score += 1;
        if(str_contains(el_id, marker))
            score += 1;
    }
    
    // If we can grab it from the meta tags, that will be our best bet
    if(strcmp(el->tag.c_str(),"meta") == 0){
        for(auto key : meta_author_markers){
            if(str_contains(el->attribute("name"),key.c_str()) || str_contains(el->attribute("property"), key.c_str())){
                author = el->attribute("content");
                score = 100;
                break;
            }
        }
        
    } else {
        score += title_tag_scores[el->tag];
        author = el->text();
        if (el->text().size() == 0) {
            score = 0;
            author = "";
        }
    }
    //maybe bias this towards the amount of content that is relevant / total content length
    if(score > 1){
        score = (score / (el->children.size()+1)) + 1;
    }
    if(body_size == 0 || body_size > 25)
        score -= 5;
    
    return make_pair(author, score);
};

pair<string, int> document_parser::score_title(html_element * el){
    int score = 0;
    string title;
    
    // If we can grab it from the meta tags, that will be our best bet
    if(strcmp(el->tag.c_str(),"meta") == 0){
        if(str_contains(el->attribute("name"),"title") || str_contains(el->attribute("property"), "title")){
            title = el->attribute("content");
            score = 100;
        }
        
    } else {
        score += title_tag_scores[el->tag];
        title = el->text();
        if (el->text().size() == 0) {
            score = 0;
            title = "";
        }
    }
    
    return make_pair(title, score);
};

pair<string, int> document_parser::score_date(html_element * el){
    string el_class = str_as_lower(el->attribute("class") ),
    el_id = str_as_lower(el->get_id()),
    date;
    size_t body_size = 0;
    for(auto & child : el->children){
        if(child.tag.size() == 0){
            body_size += child.text().size();
        }
    }
    int score = 0;
    for(auto marker : date_markers){
        if(str_contains(el_class, marker))
            score += 1;
        if(str_contains(el_id, marker))
            score += 1;
    }
    
    if(strcmp(el->tag.c_str(),"meta") == 0){
        for(auto key : meta_date_markers){
            if(str_contains(el->attribute("name"),key.c_str()) || str_contains(el->attribute("property"), key.c_str()) ){
                score = 100;
                date = el->attribute("content");
            }
        }
    } else if(strcmp(el->tag.c_str(), "time") == 0 || strcmp(el->tag.c_str(), "date") == 0){
        date = el->text();
        score += 50;
    }
    
    //maybe bias this towards the amount of content that is relevant / total content length
    if(score > 1){
        score = (score / (el->children.size()+1)) + 1;
    }
    if(body_size == 0 || body_size > 30)
        score -= 5;
    
    return make_pair(date, score);
};

int document_parser::score_element(html_element * el){
    // Prevent scoring more than once
    auto finder = score_map.find(el);
    if(finder != score_map.end()){
        return score_map[el];
    }
    
    int score = 1;
    size_t body_length = 0;
    string el_body = "";
    for(auto & child : el->children){
        if(child.tag.size() == 0){
            el_body += child.text();
        }
    }
    body_length = el_body.size();
    score += tag_scores[el->tag];
    score += str_count(el_body.c_str(), ',');
    score += min((int)(body_length / 100), 3);
    
    
    // Add weight for every non-block level element that is a child
    for (html_element & c : el->children) {
        score += tag_scores[c.tag];
    }
    
    // Weight by class
    string el_class = el->attribute("class"),
    el_id = el->get_id();
    bool id_pos = false,
    id_neg = false,
    class_pos = false,
    class_neg = false,
    attr_neg = false;
    
    for(auto cl : positive_classes){
        if(!class_pos && str_contains(el_class.c_str(), cl)) {
            class_pos = true;
        }
        if(!id_pos && str_contains(el_id.c_str(), cl)) {
            id_pos = true;
        }
    }
    for(auto cl : negative_classes){
        if(!class_neg && str_contains(el_class.c_str(), cl)) {
            class_neg = true;
        }
        if(!id_neg && str_contains(el_id.c_str(), cl)) {
            id_neg = true;
        }
    }
    
    auto attribs = el->get_attributes();
    for(auto attr : attribs){
        // if attr_first in bad_attributes
        for(auto bad_attr: bad_attributes){
            if(str_contains(attr.first, bad_attr)){
                attr_neg = true;
                break;
            }
        }
        if(attr_neg)
            break;
    }
    
    if(attr_neg)
        score -= 250;
    if(id_neg)
        score -= 25;
    if(class_neg)
        score -= 25;
    if(id_pos)
        score += 25;
    if(class_pos)
        score += 25;
    
    size_t link_length = 0;
    auto links = el->search(HTML_TAG("a"));
    for(auto & link : links) {
        link_length += link.get().text().size();
    }
    
    // It's probably either an ad or navigation
    if((float)link_length / (float)body_length > 0.25){
        score -= 25;
    }
    
    score_map[el] = score;
    
    return score;
}

document_parser::document_parser(string document_body) : root_element(document_body) {
    vector<int>indexes;
    indexes.push_back(0);
    
    html_element * c = &root_element;
    pair<html_element *, int> top_candidate;
    vector<int> top_candidate_location;
    unordered_set<html_element * > byline_candidates;
    pair<string,int> scored_title, scored_author, scored_date;
    
    //Iterate every element
    while(c){
        // 1. Score and output
        // Get the score to check if this is a better candidate than what we have marked as the top candidate
        int score = score_element(c);
        if (score > top_candidate.second) {
            //need indexes at this point?
            top_candidate = make_pair(c, score);
            top_candidate_location = indexes;
        }
        
        // We also need to mark byline candidates
        pair<string, int> current_author = score_author(c);
        if(current_author.second > scored_author.second)
            scored_author = current_author;
        
        pair<string, int> current_date = score_date(c);
        if(current_date.second > scored_date.second)
            scored_date = current_date;
        
        // Keep a backup of byline candidates in case normal scoring doesn't work
        string el_text = c->text();
        if(el_text.size() < 140 && str_contains(el_text, "by") && str_contains(el_text, "on"))
            byline_candidates.insert(c);
        
        
        //Grab the title if we don't have one
        pair<string, int> current_title = score_title(c);
        if(current_title.second > scored_title.second)
            scored_title = current_title;
        
        // Count classes to look for patterns later
        for(string el_class : c->get_classnames()){
            bool contains_bad_class = false;
            // filter out bad classnames
            for(auto bad_class : not_a_list){
                if(str_contains(el_class, bad_class)){
                    contains_bad_class = true;
                    break;
                }
            }
            if(contains_bad_class == false){
                class_counts[el_class]++;
            }
        }
        
        
        
        // 2. update the position
        
        // Move down the tree if we can
        if(c->children.size() > 0){
            indexes.push_back(0);
            c = &c->children.front();
        } else {
            // Move up the tree until we can hit a sibling
            while(c) {
                //If we have siblings
                if (c->parent != nullptr && c->parent->children.size() > ++indexes.back()) {
                    //Move to the sibling
                    c = &c->parent->children[indexes.back()];
                    break;
                } else {
                    // Else we need to move further up
                    indexes.pop_back();
                    
                    if (indexes.size() > 1 && c->parent->parent->children.size() > ++indexes.back()) {
                        // Move to aunt/uncle element
                        c = &c->parent->parent->children[indexes.back()];
                        break;
                    } else {
                        //Move to the parent and keep looking
                        // also if we're at the root, this will be nullptr, causing the loop to exit
                        c = c->parent;
                    }
                    
                    if(c != nullptr && c->parent == nullptr && c->children.size() <= indexes.back() + 1){
                        // Nothing left to read, break the loop
                        c = nullptr;
                    }
                }
            }
        }
        
    }
    
    //if top candidate has siblings that have similar scores
    html_element * top_element = top_candidate.first;
    vector<html_element *> top_candidates;
    top_candidates.push_back(top_element);
    if(top_element->parent && top_element->parent->children.size() > 1){
        for(html_element & el : top_element->parent->children){
            if(&el == top_element)
                continue;
            int score = score_element(&el);
            // This almost never happens?
            if (score >= top_candidate.second * 0.75) {
                top_candidates.push_back(&el);
            }
        }
    }
    
    //Now that we have the body elements, we need to remove anything littering the content
    // We'll just combine everything that looks like content into its own list
    for(auto & candidate : top_candidates){
        for(auto & child : candidate->children){
            if(score_element(&child) > -5){
                body_elements.push_back(&child);
            }
        }
    }
    
    // If we didn't get all the info we need out of the byline, fall back on backup candidates
    if(scored_author.first.size() == 0 || scored_date.first.size() == 0){
        for(auto candidate : byline_candidates){
            string el_text = candidate->text();
            size_t by_index = el_text.find("by "),
            on_index = el_text.find(" on");
            if(on_index == string::npos)
                on_index = el_text.find(" at");
            
            if(on_index == string::npos || by_index == string::npos)
                continue;
            
            if(by_index < el_text.size() / 2){
                if(scored_author.first.size() == 0)
                    scored_author.first = el_text.substr(by_index,on_index);
                
                if(scored_date.first.size() == 0)
                    scored_date.first = el_text.substr(on_index);
            }
        }
    }
    title = scored_title.first;
    author = scored_author.first;
    date = scored_date.first;
}

string document_parser::clean_html(){
    string output;
    // Iterate each body element to remove any ads... goddamn ads...
    for(auto & el : body_elements){
        
        vector<int>child_idxes;
        child_idxes.push_back(0);
        
        // Cleanup the element, removing anything with a negative score
        for(auto & child : el->children){
            html_element * c = &child;
            while(c){
                auto attrs = c->get_attributes();
                auto score = score_element(c);
                if(score < -5){
                    if(c->parent != nullptr){
                        auto to_remove = std::find_if(c->parent->children.begin(), c->parent->children.end(), [c](const html_element & m) { return &m == c; });
                        if(to_remove != c->parent->children.end()) {
                            
                            c->parent->children.erase( to_remove );
                        }
                    }
                }
                // 2. update the position
                
                // Move down the tree if we can
                if(c->children.size() > 0 && score > -5){
                    child_idxes.push_back(0);
                    c = &c->children.front();
                } else {
                    // Move up the tree until we can hit a sibling
                    while(c) {
                        //If we have siblings
                        if (c->parent != nullptr && c->parent->children.size() > ++child_idxes.back()) {
                            //Move to the sibling
                            c = &c->parent->children[child_idxes.back()];
                            break;
                        } else {
                            // Else we need to move further up
                            child_idxes.pop_back();
                            if(child_idxes.size() == 0){
                                break;
                            }
                            
                            if (child_idxes.size() > 1 && c->parent->parent->children.size() > ++child_idxes.back()) {
                                // Move to aunt/uncle element
                                c = &c->parent->parent->children[child_idxes.back()];
                                break;
                            } else {
                                //Move to the parent and keep looking
                                // also if we're at the root, this will be nullptr, causing the loop to exit
                                c = c->parent;
                            }
                            
                            if(c != nullptr && c->parent == nullptr && c->children.size() <= child_idxes.back() + 1){
                                // Nothing left to read, break the loop
                                c = nullptr;
                            }
                        }
                    }
                    if(child_idxes.size() == 0){
                        // We're done with this element
                        child_idxes.push_back(0);
                        break;
                    }
                }
                
            }
        }
        
        // add our newly cleaned element to the output
        output += el->text(html_element::safe_tags, html_element::safe_attributes, true, true);
    }
    
    return output;
}

vector<string> document_parser::list_items(){
    vector<string> output;
    pair<string,float> max_density;
    for(auto el_class : class_counts){
        auto elements = root_element.search(HTML_CLASS(el_class.first.c_str()));
        // We only care about classes that repetitively show up
        if(elements.size() > 3){
            
            //We don't really care which siblings share the class, just that there's a high desnity
            unordered_map<html_element *, int> sibling_counts;
            int max_sibling_count = 0;
            for(html_element & el : elements){
                //only use elements with content
                if(el.text().size() > 0){
                    sibling_counts[el.parent]++;
                    if(sibling_counts[el.parent] > max_sibling_count)
                        max_sibling_count = sibling_counts[el.parent];
                }
            }
            
            // If this has the highest density, store it
            float density = (float)max_sibling_count / (float)elements.size();
            if(density > max_density.second){
                // Store the density
                max_density = make_pair(el_class.first,density);
            }
        }
    }
    
    for(html_element & list_element : root_element.search(HTML_CLASS(max_density.first.c_str()))) {
        output.push_back(list_element.text(html_element::safe_tags, html_element::safe_attributes, true, true));
    }
    return output;
}


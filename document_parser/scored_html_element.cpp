//
//  scored_html_element.cpp
//  document_parser
//
//  Created by Chris Pauley on 3/12/17.
//  Copyright © 2017 Infegy. All rights reserved.
//

#include "scored_html_element.h"


scored_html_element::scored_html_element(string to_parse) {
    _par
    _build_score();
}
/*scored_html_element::scored_html_element(html_element base): html_element(base) {
    _build_score();
}*/

void scored_html_element::_build_score(){
    score = 1;
    size_t body_length = 0;
    string el_body = "";
    for(auto & child : children){
        if(child.tag.size() == 0){
            el_body += child.text();
        }
    }
    body_length = el_body.size();
    score += tag_scores[tag];
    score += str_count(el_body.c_str(), ',');
    score += min((int)(body_length / 100), 3);
    
    
    // Add weight for every non-block level element that is a child
    for (html_element & c : children) {
        score += tag_scores[c.tag];
    }
    
    // Weight by class
    string el_class = attribute("class"),
    el_id = get_id();
    
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
    
    auto attribs = get_attributes();
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
    
    // Discount high link density
    size_t link_length = 0;
    auto links = find_all_tags("a");
    for(auto & link : links) {
        link_length += link->text().size();
    }
    
    // It's probably either an ad or navigation
    if((float)link_length / (float)body_length > 0.25){
        score -= 25;
    }
}

pair<string, int> scored_html_element::title_score() {
    int title_score = 0;
    string title;
    
    // If we can grab it from the meta tags, that will be our best bet
    if(strcmp(tag.c_str(),"meta") == 0){
        if(str_contains(attribute("name"),"title") || str_contains(attribute("property"), "title")){
            title = attribute("content");
            title_score = 100;
        }
        
    } else {
        title_score += title_tag_scores[tag];
        title = text();
        if (title.size() == 0) {
            title_score = 0;
            title = "";
        }
    }
    
    return make_pair(title, score);
}

pair<string, int> scored_html_element::author_score() {
    string el_class = str_as_lower( attribute("class") ),
    el_id = str_as_lower( get_id()),
    author;
    size_t body_size = 0;
    for(auto & child : children){
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
    if(strcmp(tag.c_str(),"meta") == 0){
        for(auto key : meta_author_markers){
            if(str_contains(attribute("name"),key.c_str()) || str_contains(attribute("property"), key.c_str())){
                author = attribute("content");
                score = 100;
                break;
            }
        }
        
    } else {
        score += title_tag_scores[tag];
        author = text();
        if (text().size() == 0) {
            score = 0;
            author = "";
        }
    }
    //maybe bias this towards the amount of content that is relevant / total content length
    if(score > 1){
        score = (score / (children.size()+1)) + 1;
    }
    if(body_size == 0 || body_size > 25)
        score -= 5;
    
    return make_pair(author, score);
}

pair<string, int> scored_html_element::date_score() {
    string el_class = str_as_lower(attribute("class") ),
            el_id = str_as_lower(get_id()),
            date;
    size_t body_size = 0;
    int date_score = 0;
    
    for(auto & child : children){
        if(child.tag.size() == 0){
            body_size += child.text().size();
        }
    }
    
    for(auto marker : date_markers){
        if(str_contains(el_class, marker))
            score += 1;
        if(str_contains(el_id, marker))
            score += 1;
    }
    
    if(strcmp(tag.c_str(),"meta") == 0){
        for(auto key : meta_date_markers){
            if(str_contains(attribute("name"),key.c_str()) || str_contains(attribute("property"), key.c_str()) ){
                score = 100;
                date = attribute("content");
            }
        }
    } else if(strcmp(tag.c_str(), "time") == 0 || strcmp(tag.c_str(), "date") == 0){
        date = text();
        score += 50;
    }
    
    //maybe bias this towards the amount of content that is relevant / total content length
    if(date_score > 1){
        date_score = (date_score / (children.size()+1)) + 1;
    }
    if(body_size == 0 || body_size > 30)
        score -= 5;
    
    return make_pair(date, score);
}

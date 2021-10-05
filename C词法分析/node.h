#pragma once
#include <iostream>
class node
{
private:
    char ch;
    node* peer;
    node* son;
public:
    node(char ch) {
        this->ch = ch;
        peer = NULL;
        son = NULL;
    }
    ~node() {
        delete peer;
        delete son;
    }
    node* newSon(char ch) {
        node* temp = new node(ch);
        temp->peer = this->son;
        son = temp;
        return temp;
    }
    node* newPeer(char ch) {
        node* temp = new node(ch);
        temp->peer = this->peer;
        this->peer = temp;
        return temp;
    }
    bool hasSon() { return son != NULL; }
    bool hasPeer() { return peer != NULL; }
    node* Son() { return son; }
    node* Peer() { return peer; }
    node* seekPeer(char charac) {
        if (this->ch == charac)return this;
        for (node* now = peer; now != NULL; now = now->peer) {
            if (now->ch == charac)return now;
        }return NULL;
    }
    node* seekSon(char charac) {
        return this->hasSon() ? this->Son()->seekPeer(charac) : NULL;
    }
};

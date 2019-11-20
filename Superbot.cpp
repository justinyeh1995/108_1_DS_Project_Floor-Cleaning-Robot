#include <iostream>
#include <fstream>
#include "Stack.h"
#include "Queue.h"

using namespace std;

struct NodeItem{
    int row;
    int col;
    int weight; // weight = abs(x-root.row) + abs(y-root.y)
    NodeItem* parent; // used in shortest-path
    bool hasShortestpath;
    int shorteststeps;
    NodeItem(int r, int c, int d):row(r),col(c),weight(d),parent(0),hasShortestpath(false){}
    NodeItem operator=(NodeItem* n) {
        this->row = n->row;
        this->col = n->col;
        this->weight = n->weight;
        this->parent = n->parent;
        return *this;
    } 
};

class Robot{
    private:
        NodeItem* root;
        int num_node;
        int row, col, battery;
        int** map; // map to store where has been visited
        bool** mapSP; // original bool map
        bool** mapcopy; // used in BFS_ShortestPath
        int** partial; // an array to store the # of zeros on a path
        int** step_to_root;
        NodeItem** pathlookup; // an array to store the resulting path 
        stack<NodeItem*> track;// a stack to store the last step
        queue<NodeItem*> step; // the path itself

    public:
        //constructor; initinalize map, battery life, num_node and root node 
        Robot(int r, int c, int b, char** input):row(r),col(c),battery(b){
            // construct map and all the other tools
            map = new int*[r];
            mapSP = new bool*[r];
            mapcopy = new bool*[r];
            step_to_root = new int*[r];
            pathlookup = new NodeItem*[r*c];
            for(int i =0; i < r; i++)
            {
                map[i] = new int[c];
                mapSP[i] = new bool[c];
                mapcopy[i] = new bool[c];
                step_to_root[i] = new int[c];
            }
            // mapping input & map and count nodes
            num_node =0;
            for(int i = 0; i < r; i++) 
            {
                for(int j = 0; j < c; j++)
                {
                    if(input[i][j]=='R')
                    {
                        map[i][j] = 1;
                        mapSP[i][j] = true;
                        root = new NodeItem(i, j, 0);
                        track.push(root); // initialize the position 
                        step.push(root);  // first step is root 
                    }
                    else if(input[i][j]=='1'){
                        map[i][j] = 2;
                        mapSP[i][j] = false;
                    }
                    else{
                        map[i][j] = 0;
                        num_node++; // count zeros
                        mapSP[i][j] = true;
                    }
                }
            }
            // The initial states of the minimum steps to each zero
            partial = new int*[row];
            for (int i  = 0; i < row; i++) {
                partial[i] = new int[col];
            }
            for (int i  = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                    partial[i][j] = 0;
                    step_to_root[i][j] = 0;
                }
            }
            // bestTravelInit(root);
            
            cout<<"num_node: "<<num_node<<"\nRoot: ("<<track.top()->row<<", "<<track.top()->col<<")"<<" track.size(): "<<track.size()<<endl;
            /*
            1 1 1 1         2 2 2 2 
            1 0 0 1   ===>  2 0 0 2  ===> num_node = 4, not including root
            1 1 0 1         2 2 0 2
            1 0 R 1         2 0 1 2   
            */
        }
        int get_num_node() {return num_node;};
        void mapCopyInit();
        bool AllClean();
        void Move();
        void Move_Large();
        void PrintStep();
        void PrintMap();
        NodeItem* ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to);
        bool Deadend(NodeItem* top);
        bool isValidStep(NodeItem*,int);
        void bestTravelInit();
        void countStepsInit();
        NodeItem* bestTravel(NodeItem* current);
        int countZeros(NodeItem* path, NodeItem* from, NodeItem* to);
        int countSteps(NodeItem* path, NodeItem* from, NodeItem* to);
        void pushSteps(NodeItem* path, NodeItem* from, NodeItem* to);
        void pushSteps_toRoot(NodeItem* temp, NodeItem* from, NodeItem* to);
        int steps() { return step.size()-1; }
        void outStep(ofstream&);
};

bool Robot::Deadend(NodeItem* top) {
     int count = 0;
     int r = top->row;
     int c = top->col;
     if(r-1 >= 0 && map[r-1][c] == 0) count++;
     if(r+1 <= row-1 && map[r+1][c] == 0 ) count++;
     if(c-1 >= 0 && map[r][c-1] == 0 ) count++;
     if(c+1 <= col-1 && map[r][c+1] == 0) count++;
     
     if(count) return false;
     else return true;
 }

void Robot::mapCopyInit() {
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            mapcopy[i][j] = mapSP[i][j];
        }
    }
}

// shortest but also record it //return a linked-list
NodeItem* Robot::ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to) { 
    mapCopyInit();
    queue<NodeItem*> bfs;
    stack<NodeItem*> s;
    bfs.push(from);
    NodeItem* ans = NULL;
    while(!bfs.empty()) {
        NodeItem* current = bfs.front();
        int r = current->row;
        int c = current->col;
        mapcopy[r][c] = false;
        bfs.pop();        
        // case 0 : from == to
        if(r == to->row && c == to->col) { ans = current; break;}
        
        int weightUp = abs(r-1 - root->row)+ abs(c - root->col);
        int weightDown = abs(r+1 - root->row) + abs(c - root->col);
        int weightLeft = abs(r - root->row) + abs(c-1 - root->col);
        int weightRight = abs(r - root->row) + abs(c+1 - root->col);

        if(r-1 >= 0 && mapcopy[r-1][c] == true ) { NodeItem* up = new NodeItem(r-1, c, weightUp); mapcopy[r-1][c] =false; up->parent = current;  bfs.push(up);  if(r-1 == to->row && c == to->col) {ans = up; break;}}
        if(r+1 <= row-1 && mapcopy[r+1][c] == true) { NodeItem* down = new NodeItem(r+1, c, weightDown); mapcopy[r+1][c] = false; down->parent = current;  bfs.push(down);;if(r+1 == to->row && c == to->col) {ans = down; break;}} 
        if(c-1 >= 0 && mapcopy[r][c-1] == true ) { NodeItem* left = new NodeItem(r, c-1, weightLeft); mapcopy[r][c-1] = false; left->parent = current;  bfs.push(left); if(r == to->row && c-1 == to->col) {ans = left; break;}}
        if(c+1 <= col-1 && mapcopy[r][c+1] == true) { NodeItem* right = new NodeItem(r, c+1, weightRight); mapcopy[r][c+1] = false; right->parent = current;  bfs.push(right); if(r == to->row && c+1 == to->col){ans = right; break;}}
    }
    cout<<endl;
    ans->hasShortestpath = true;
    return ans;
}

int Robot::countZeros(NodeItem* path, NodeItem* from, NodeItem* to) {  
    int num_of_zero = 0;

    int r = 0;
    int c = 0;
    while(path != from) {
        r = path->row;
        c = path->col;
        if (map[r][c] == 0) {
            num_of_zero++;
        }
        path = path->parent; 
    }
    return num_of_zero;
}

int Robot::countSteps(NodeItem* check, NodeItem* from, NodeItem* to) {
    int steps = 0;
    NodeItem* tmp = check;
    int r = 0;
    int c = 0;
    while(tmp != from) {
        tmp = tmp->parent;
        steps++;
    }
    return steps;
}

void Robot::bestTravelInit() {
    NodeItem* target;
    // partial[i][j] stores how many 0s is on the shortest path to a certain position
    // get every partial[i][j]
    for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (map[i][j] == 0) {
                // if(pathlookup[i*col + j+1]->hasShortestpath)
                target = new NodeItem(i,j,0);
                NodeItem* path = ShortestPath_to_R_BFS(root, target);
                pathlookup[i*col + j] = path;
                int zero = countZeros(path, root, target);
                partial[i][j] = zero;
            }    
        }
    }
}

void Robot::countStepsInit() {
     NodeItem* target;
     for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (map[i][j] == 0) {
                target = new NodeItem(i,j,0);
                NodeItem* path = ShortestPath_to_R_BFS(root, target);
                pathlookup[i*col + j] = path;
                step_to_root[i][j] = countSteps(path,root,target);
                cout<<step_to_root[i][j]<<endl;
            }
        }
     }
}

NodeItem* Robot::bestTravel(NodeItem* current) {
    // return the best option
    int best = 0;
    int x = 0;
    int y = 0; 
    if( num_node <= 11000) {
        for (int i  = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                if (map[i][j] == 0 ) {
                    // cout<<"comparing"<<endl;
                    partial[i][j] = countZeros(pathlookup[i*col+j], root, root);
                    if(partial[i][j] > best ) {
                    best = partial[i][j];
                    x = i;
                    y = j;
                    }
                } 
            }
        }
    }
    else {
        for (int i  = 0; i < row; i++) {
        for (int j = 0; j < col; j++) {
            if (map[i][j] == 0 ) {
                x = i;
                y = j;
                }
            } 
        }
    }
    NodeItem* bestoption = new NodeItem(x,y,0);
    //cout<<"how many zeros on path: "<<best<<endl;
    return bestoption;
}

bool Robot::AllClean() {
    for(int i = 0; i < row; i++) 
    {
        for(int j = 0; j < col; j++)
        {
            if(map[i][j] == 0) return false;
        }
    }
    return true;
}

bool Robot::isValidStep(NodeItem* now, int b) {
    
    stack<NodeItem*> stk;
    int r = now->row;
    int c = now->col;
    int w = now->weight;
    NodeItem* next;
    // Move Up Valid
    if(r-1 >= 0 && map[r-1][c] == 0 ) {
        next = new NodeItem(r-1,c,w+1);
        NodeItem* check = ShortestPath_to_R_BFS(next, root);
        int toRoot = 0;
        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    // Move Down Valid
    else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
        next = new NodeItem(r+1,c,w+1);
        NodeItem* check = ShortestPath_to_R_BFS(next, root);
        int toRoot = 0;

        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    //Move Left Valid
    else if(c-1 >= 0 && map[r][c-1] == 0 ) {
        next = new NodeItem(r,c-1,w+1);
        NodeItem* check = ShortestPath_to_R_BFS(next, root);
        int toRoot = 0;

        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    //Move Right Valid
    else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
        next = new NodeItem(r,c+1,w+1);
        NodeItem* check = ShortestPath_to_R_BFS(next, root);
        int toRoot = 0;
        while(!stk.empty()) {
            stk.pop();
        }
        while(check != next) {
            toRoot++;
            check = check->parent;
        }
        if(b-toRoot >0)return true;
    }
    return false;
}

void Robot::pushSteps(NodeItem* temp, NodeItem* from, NodeItem* to) {
    stack<NodeItem*> s;
    while (!s.empty()) {s.pop();}
    int r = 0;
    int c = 0;
    while(temp != from) {
        s.push(temp);
        temp = temp->parent;
    } 
    while (!s.empty()) {
        step.push(s.top());
        r = s.top()->row;
        c = s.top()->col; 
        map[r][c] = 1;
        track.push(s.top());
        cout<<"[ "<< s.top()->row<<", "<<s.top()->col<<" ]"<<endl;
        s.pop();
    }
}

void Robot::pushSteps_toRoot(NodeItem* temp, NodeItem* from, NodeItem* to) {
    stack<NodeItem*> s;
    while (!s.empty()) {s.pop();}
    int r = 0;
    int c = 0;
    while(temp != from) {
        temp = temp->parent;
        r= temp->row;
        c = temp->col;
        step.push(temp);
        map[r][c] = 1;
        cout<<"[ "<< r<<", "<<c<<" ]"<<endl;
    }
    // step.push(to); 
}

void Robot::Move() {
    stack<NodeItem*> s3;
    int batterylife = battery;

    while(!AllClean()) 
    {
        
        cout<<"Energy now: "<<batterylife<<endl;
        if(batterylife < 0) break;
        NodeItem* now = track.top();
        cout<<"Now position: "<<"[ "<< now->row<<", "<<now->col<<" ]"<<endl;
        int r = now->row;
        int c = now->col;
        int w = now->weight;
        

        // Normal situation: battery is still enough 
        if (isValidStep(now, batterylife) ) {
            cout<<"GO! "<<endl;
            NodeItem* newnode;
            // Moving up
            if(r-1 >= 0 && map[r-1][c] == 0 ) {
                //visiting
                newnode = new NodeItem(r-1, c, w+1);
                map[r-1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving down
            else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
                //visiting
                newnode = new NodeItem(r+1, c, w+1);
                map[r+1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving Left
            else if(c-1 >= 0 && map[r][c-1] == 0 ) {
                //visiting
                newnode = new NodeItem(r, c-1, w+1);
                map[r][c-1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;    
            }
            // Moving Right
            else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
                //visiting
                newnode = new NodeItem(r, c+1, w+1);
                map[r][c+1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            
        }
        // Dead-End: 1. Search for the node that has unvisited adjancey node(s)
        else if ( Deadend(track.top()) ) {
            cout<<"DeadEnd Occurs"<<endl;
            NodeItem* from = track.top();
            cout<<"From the dead-end node: "<<"[ "<< from->row<<", "<<from->col<<" ]"<<endl;
            while(Deadend(track.top())) {
                track.pop();
            }
            NodeItem* to = track.top();
            
            cout<<"Back to the node with unvisited adjancey nodes: "<<"[ "<< to->row<<", "<<to->col<<" ]"<<endl;
            //cout<<endl;            
            NodeItem* temp = ShortestPath_to_R_BFS(from, to);
            int counts = countSteps(temp, from,  to);
            cout<<"counts: "<<counts<<endl;
            cout<<endl;
            // if (to == root) then goRoot->parent == NULLs
            NodeItem* temps = ShortestPath_to_R_BFS(to, root);
            int counting = countSteps(temps, to, root);

            
            // it is safe to go 
            if((batterylife - counts - counting  -2) >= 0) {
                // cout<<"Safe to go!"<<endl;
                pushSteps(temp, from,  to); 
                batterylife -= counts;
            }
            //  need to recharge first
            else {
                //cout<<"\nRecharge: "<<(counts+counting)<<" >= "<<batterylife<<endl;
                /*Recharge*/
                NodeItem*tempss = ShortestPath_to_R_BFS(from, root);
                pushSteps(tempss, from, root); 
                batterylife = battery;
                /*go to the best path*/
                NodeItem* best = bestTravel(root);
                cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
                NodeItem* newplace = ShortestPath_to_R_BFS(root, best);
                int steps = countSteps(newplace, root, best);
                pushSteps(newplace, root , best);
                track.push(best);
                batterylife -= steps;
            }
        }
        // Rechrge time: 
        else {
            cout<<"Recharge time!"<<endl;
            /* Calculate the shortest path from now to root */
            NodeItem* check = ShortestPath_to_R_BFS(now, root);
            pushSteps(check, now,  root); 
            batterylife = battery;
            //cout<<"Now at: "<<"[ "<<track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
            //cout<<"energy: "<<batterylife<<endl;
            NodeItem* best = bestTravel(track.top());
            cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
            NodeItem*temp = ShortestPath_to_R_BFS(track.top(), best);
            int steps = countSteps(temp, track.top(), best); // top == root
            pushSteps(temp, track.top(), best); 
            batterylife -= steps;
        }
        //  PrintMap();
        //  cout<<"step: "<<step.size()-1<<endl;
        //  cout<<endl;      
    }
    /*
    After all cleaned, walk shortestPath back to root
    */
    cout<<"Timetogohome! now at position: "<<"[ "<< track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
    NodeItem* temp = ShortestPath_to_R_BFS(track.top(), root);
    while(temp != track.top()) {
        s3.push(temp);
        temp = temp->parent;
    }
    while(!s3.empty()) {
        step.push(s3.top());
        //cout<<"[ "<< s3.top()->row<<", "<<s3.top()->col<<" ]"<<endl;
        s3.pop();
    }
    cout<<"Total: "<<steps()<<endl;
}

void Robot::Move_Large() {
    int batterylife = battery;
    int rx, ry;
   
        // go to the furthest
        for(int i = 0; i < row; i++) {
            for(int j = 0; j < col; j++) {
                if(map[i][j] == 0) {
                    // int count = countSteps(pathlookup[i*col+j],root,root);
                    cout<<"Go"<<endl;
                    pushSteps(pathlookup[i*col+j],root,root);
                    batterylife -= step_to_root[i][j];
                    int x = i;
                    int y = j;
                    while(batterylife - step_to_root[x][y] >= 2) {
                        //cout<<"( "<<x<<","<<y<<" )"<<endl;
                        cout<<"Wander"<<endl;
                        NodeItem* temp;
                        if(map[x+1][y] == 0) {cout<<"( "<<pathlookup[(x+1)*col+y]->row<<","<<pathlookup[(x+1)*col+y]->col<<" )"<<endl; batterylife--; step.push(pathlookup[(x+1)*col+y]); map[x+1][y] = 1; x++;}
                        else if(map[x][y+1] == 0) {cout<<"( "<<pathlookup[(x)*col+y+1]->row<<","<<pathlookup[(x)*col+y+1]->col<<" )"<<endl; batterylife--; step.push(pathlookup[(x)*col+y+1]);map[x][y+1] = 1;y++;}
                        else if(map[x-1][y] == 0) {cout<<"( "<<pathlookup[(x-1)*col+y]->row<<","<<pathlookup[(x-1)*col+y]->col<<" )"<<endl; batterylife--; step.push(pathlookup[(x-1)*col+y]);map[x-1][y] = 1;x--;}
                        else if(map[x][y-1] == 0) {cout<<"( "<<pathlookup[(x)*col+y-1]->row<<","<<pathlookup[(x)*col+y-1]->col<<" )"<<endl; batterylife--; step.push(pathlookup[(x)*col+y-1]);map[x][y-1] = 1; y--;}
                        else {cout<<"DeadEnd"<<endl; break;}
                    }
                    cout<<"Recharge"<<endl;
                    pushSteps_toRoot(pathlookup[x*col+y],root, pathlookup[x*col+y]);
                    batterylife = battery;
                }
            }
        }
    
    cout<<"All Cleaned"<<endl;
}

void Robot::outStep(ofstream& outFile) {
    while(!step.empty())
    {
        outFile<<step.front()->row<<" "<<step.front()->col<<endl;
        step.pop();
    }
}

void Robot::PrintStep() {
    while(!step.empty())
    {
        cout<<step.front()->row<<" "<<step.front()->col<<endl;
        step.pop();
    }
}

void Robot::PrintMap(){
    for(int i = 0; i < row; i++)
    {
        for(int j = 0; j < col;j++)
        {
            cout<<map[i][j];
        }
        cout<<endl;
    }
}

int main() {

    ifstream file("floor.data");
    int r,c,b;
    file>>r>>c>>b;
    char** input = new char*[r];
    for(int i = 0; i < r; i++)
    {
        input[i] = new char[c];
    }
    for(int i = 0; i < r; i++)
    {
        for(int j = 0; j < c;j++)
        {
            file>>input[i][j];
        }
    }
    Robot robot(r,c,b,input);
    int flag = robot.get_num_node();
    if(flag <= 11000) {
        robot.bestTravelInit();
        robot.Move();
    }
    else if(flag > 20000) {
        cout<<"\nLarge"<<endl;
        robot.countStepsInit();
        robot.Move_Large();
    }
    else {
        cout<<"Medium"<<endl;
        robot.Move();
    }
    
    ofstream outFile("final.path", ios::out); 
    outFile<<robot.steps()<<endl;
    robot.PrintMap();
    //robot.PrintStep();
    robot.outStep(outFile);
}


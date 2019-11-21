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
    NodeItem(int r, int c, int d):row(r),col(c),weight(d),parent(0){}
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
        stack<NodeItem*> track;// a stack to store the last step
        queue<NodeItem*> step; // the path itself
        NodeItem*** path;
        

    public:
        //constructor; initinalize map, battery life, num_node and root node 
        Robot(int r, int c, int b, char** input):row(r),col(c),battery(b){
            // construct map and all the other tools
            map = new int*[r];
            mapSP = new bool*[r];
            mapcopy = new bool*[r];
            for(int i =0; i < r; i++)
            {
                map[i] = new int[c];
                mapSP[i] = new bool[c];
                mapcopy[i] = new bool[c];
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
            partial = new int*[row];
            for (int i  = 0; i < row; i++) {
                partial[i] = new int[col];
            }
            for (int i  = 0; i < row; i++) {
                for (int j = 0; j < col; j++) {
                    partial[i][j] = 0;
                }
            }
            // bestTravelInit(root);
            ShortestPath_SpanningMap(root);
            //cout<<"num_node: "<<num_node<<"\nRoot: ("<<track.top()->row<<", "<<track.top()->col<<")"<<" track.size(): "<<track.size()<<endl;
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
        NodeItem*** ShortestPath_SpanningMap(NodeItem* root);
        NodeItem* ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to);
        bool Deadend(NodeItem* top);
        bool isValidStep(NodeItem*,int);
        void bestTravelInit();
        void countStepsInit();
        NodeItem* bestTravel(NodeItem* current);
        int countZeros(int i, int j);
        int countSteps_to_from(NodeItem* check, NodeItem* from, NodeItem* to);
        int countSteps(int i , int j);
        void pushSteps(NodeItem* path, NodeItem* from);
        void pushSteps_to_from(NodeItem* temp, NodeItem* from, NodeItem* to); 
        void pushSteps_toRoot(NodeItem* temp, NodeItem* from);
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


// shortest but also record it //return a linked-list
NodeItem*** Robot::ShortestPath_SpanningMap(NodeItem* from) { 
    mapCopyInit();
    path = new NodeItem**[row];
    for(int i = 0; i < row; i++) {
        path[i] = new NodeItem*[col];
    }
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            path[i][j] = NULL;
        }
    }
    path[root->row][root->col] = root;
    path[root->row][root->col]->weight = 0;
    queue<NodeItem*> bfs;
    stack<NodeItem*> s;
    bfs.push(from);
    while(!bfs.empty()) {
        NodeItem* current = bfs.front();
        int r = current->row;
        int c = current->col;
        mapcopy[r][c] = false;
        bfs.pop();        
        //cout<<"mapcopy[r-1][c]: "<<mapcopy[r-1][c]<<endl;
        if(r-1 >= 0 && mapcopy[r-1][c] == true ) { NodeItem* up = new NodeItem(r-1, c, current->weight+1); mapcopy[r-1][c] =false; up->parent = current; path[r-1][c] = up; bfs.push(up); }
        if(r+1 <= row-1 && mapcopy[r+1][c] == true) { NodeItem* down = new NodeItem(r+1, c, current->weight+1); mapcopy[r+1][c] = false; down->parent = current;  path[r+1][c] = down; bfs.push(down);} 
        if(c-1 >= 0 && mapcopy[r][c-1] == true ) { NodeItem* left = new NodeItem(r, c-1, current->weight+1); mapcopy[r][c-1] = false; left->parent = current; path[r][c-1] = left; bfs.push(left);}
        if(c+1 <= col-1 && mapcopy[r][c+1] == true) { NodeItem* right = new NodeItem(r, c+1, current->weight+1); mapcopy[r][c+1] = false; right->parent = current; path[r][c+1] = right; bfs.push(right);}
    } 
    return path;
}
void Robot::mapCopyInit() {
    for(int i = 0; i < row; i++) {
        for(int j = 0; j < col; j++) {
            mapcopy[i][j] = mapSP[i][j];
        }
    }
}
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

        int weightUp = 0;
        int weightDown = 0;
        int weightLeft = 0;
        int weightRight = 0;

        if(r-1 >= 0 && mapcopy[r-1][c] == true ) { NodeItem* up = new NodeItem(r-1, c, weightUp); mapcopy[r-1][c] =false; up->parent = current;  bfs.push(up);  if(r-1 == to->row && c == to->col) {ans = up; break;}}
        if(r+1 <= row-1 && mapcopy[r+1][c] == true) { NodeItem* down = new NodeItem(r+1, c, weightDown); mapcopy[r+1][c] = false; down->parent = current;  bfs.push(down);;if(r+1 == to->row && c == to->col) {ans = down; break;}} 
        if(c-1 >= 0 && mapcopy[r][c-1] == true ) { NodeItem* left = new NodeItem(r, c-1, weightLeft); mapcopy[r][c-1] = false; left->parent = current;  bfs.push(left); if(r == to->row && c-1 == to->col) {ans = left; break;}}
        if(c+1 <= col-1 && mapcopy[r][c+1] == true) { NodeItem* right = new NodeItem(r, c+1, weightRight); mapcopy[r][c+1] = false; right->parent = current;  bfs.push(right); if(r == to->row && c+1 == to->col){ans = right; break;}}
    }
    cout<<endl;
    return ans;
}
int Robot::countZeros(int i, int j) {  
    int num_of_zero = 0;
    int r = 0;
    int c = 0;
    NodeItem* temp = path[i][j]; 
    while(temp!= root) {
        r = temp->row;
        c = temp->col;
        if (map[r][c] == 0) {
            num_of_zero++;
        }
        temp = temp->parent; 
    }
    return num_of_zero;
}

int Robot::countSteps(int i , int j) {
    return path[i][j]->weight;
}

int Robot::countSteps_to_from(NodeItem* check, NodeItem* from, NodeItem* to) {
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

NodeItem* Robot::bestTravel(NodeItem* current) {
    // return the best option
    int best = 0;
    int x = root->row;
    int y = root->col; 
     if( num_node <= 700000) {
        for (int i  = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                if (map[i][j] == 0 ) {
                    // cout<<"comparing"<<endl;
                    partial[i][j] = countZeros(i,j);
                    // cout<<partial[i][j]<<endl;
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
        
        int toRoot = path[r-1][c]->weight;
        
        if(b-toRoot >0)return true;
    }
    // Move Down Valid
    else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
        
        int toRoot = path[r+1][c]->weight;
        if(b-toRoot >0)return true;
    }
    //Move Left Valid
    else if(c-1 >= 0 && map[r][c-1] == 0 ) {
        int toRoot = path[r][c-1]->weight;
        if(b-toRoot >0)return true;
    }
    //Move Right Valid
    else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
        int toRoot = path[r][c+1]->weight;
        if(b-toRoot >0)return true;
    }
    return false;
}

void Robot::pushSteps(NodeItem* temp, NodeItem* from) {
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
        //cout<<"[ "<< s.top()->row<<", "<<s.top()->col<<" ]"<<endl;
        s.pop();
    }
}
void Robot::pushSteps_to_from(NodeItem* temp, NodeItem* from, NodeItem* to) {
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
        //cout<<"[ "<< s.top()->row<<", "<<s.top()->col<<" ]"<<endl;
        s.pop();
    }
}
void Robot::pushSteps_toRoot(NodeItem* temp, NodeItem* from) {
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
        //cout<<"[ "<< r<<", "<<c<<" ]"<<endl;
    }
}

void Robot::Move() {
    stack<NodeItem*> s3;
    int batterylife = battery;

    while(!AllClean()) 
    {
        
        //cout<<"Energy now: "<<batterylife<<endl;
        if(batterylife < 0) break;
        NodeItem* now = track.top();
        //cout<<"Now position: "<<"[ "<< now->row<<", "<<now->col<<" ]"<<endl;
        int r = now->row;
        int c = now->col;
        int w = now->weight;
        
        if(track.size()==1 && step.size()>1) {
            int x,y;
            for(int i = 0; i < row; i++) {
                for(int j = 0; j < col; j++) {
                    if(map[i][j] == 0) {
                        x = i;
                        y = j;
                        break;
                    }
                }
            }
            NodeItem* temp = path[x][y];
            pushSteps(temp,root);
        }

        // Normal situation: battery is still enough 
        else if (isValidStep(now, batterylife) ) {
            //cout<<"GO! "<<endl;
            NodeItem* newnode;
            // Moving up
            if(r-1 >= 0 && map[r-1][c] == 0 ) {
                //visiting
                newnode = path[r-1][c];
                map[r-1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving down
            else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
                //visiting
                newnode = path[r+1][c];
                map[r+1][c] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            // Moving Left
            else if(c-1 >= 0 && map[r][c-1] == 0 ) {
                //visiting
                newnode = path[r][c-1];
                map[r][c-1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;    
            }
            // Moving Right
            else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
                //visiting
                newnode = path[r][c+1];
                map[r][c+1] = 1;
                track.push(newnode);
                step.push(newnode);
                batterylife--;
            }
            
        }
        // Dead-End: 1. Search for the node that has unvisited adjancey node(s)
        else if ( Deadend(track.top()) ) {
            //cout<<"DeadEnd Occurs"<<endl;
            NodeItem* from = track.top();
            //cout<<"From the dead-end node: "<<"[ "<< from->row<<", "<<from->col<<" ]"<<endl;
            while(Deadend(track.top())&& track.size()>1) {
                track.pop();
            }
            // if(track.size()==1)break; 
            NodeItem* to = track.top();
            //cout<<"Back to the node with unvisited adjancey nodes: "<<"[ "<< to->row<<", "<<to->col<<" ]"<<endl;
            //cout<<endl;            
            NodeItem* temp = ShortestPath_to_R_BFS(from, to);
            int counts = countSteps_to_from(temp, from,to);
            //cout<<"counts: "<<counts<<endl;
            cout<<endl;
            // if (to == root) then goRoot->parent == NULLs
            int counting = path[to->row][to->col]->weight;

            
            // it is safe to go 
            if((batterylife - counts - counting  -2) >= 0) {
                //cout<<"Safe to go!"<<endl;
                pushSteps_to_from(temp, from,  to); 
                batterylife -= counts;
            }
            //  need to recharge first
            else {
                //cout<<"\nRecharge: "<<(counts+counting)<<" >= "<<batterylife<<endl;
                /*Recharge*/
                // NodeItem*tempss = ShortestPath_to_R_BFS(from, root);
                pushSteps_toRoot(path[from->row][from->col], root); 
                batterylife = battery;
                /*go to the best path*/
                NodeItem* best = bestTravel(root);
                //cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
                //NodeItem* newplace = ShortestPath_to_R_BFS(root, best);
                int steps = countSteps(best->row,best->col);
                pushSteps(path[best->row][best->col], root);
                track.push(best);
                batterylife -= steps;
            }
        }
        // Rechrge time: 
        else {
            //cout<<"Recharge time!"<<endl;
            /* Calculate the shortest path from now to root */
            //NodeItem* check = ShortestPath_to_R_BFS(now, root);
            pushSteps_toRoot(path[now->row][now->col], root); 
            batterylife = battery;
            //cout<<"Now at: "<<"[ "<<track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
            //cout<<"energy: "<<batterylife<<endl;
            NodeItem* best = bestTravel(track.top());
            //cout<<"go to the best node after recharge: "<<"[ "<< best->row<<", "<<best->col<<" ]"<<endl;
            //NodeItem*temp = ShortestPath_to_R_BFS(track.top(), best);
            int steps = countSteps(best->row, best->col); // top == root
            pushSteps(path[best->row][best->col], root); 
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
    pushSteps_toRoot(path[track.top()->row][track.top()->col], root); 
    cout<<"Total: "<<steps()<<endl;
}

void Robot::Move_Large() {
    int batterylife = battery;
    int rx, ry;
        // cout<<"( "<<path[1][12]->row<<","<<path[1][12]->col<<" )"<<endl;
        // go to the furthest
        for(int i = 0; i < row; i++) {
            for(int j = 0; j < col; j++) {
                if(map[i][j] == 0) {
                    // int count = countSteps(pathlookup[i*col+j],root,root);
                    //cout<<"Go"<<endl;
                    NodeItem* temp = path[i][j];
                    pushSteps(temp,root);
                    // step_to_root[i][j] = countSteps(pathlookup[i*col+j],root,0);
                    int s = path[i][j]->weight;
                    batterylife -= s;
                    int x = i;
                    int y = j;

                    while(batterylife - s >= 2) {
                        // cout<<"batterylife: "<<batterylife<<endl;
                        // cout<<"Wander"<<endl;
                        
                        if(map[x+1][y] == 0) {batterylife--; step.push(path[x+1][y]); map[x+1][y] = 1; x++;}
                        else if(map[x][y+1] == 0) { batterylife--; step.push(path[x][y+1]);map[x][y+1] = 1;y++;}
                        else if(map[x-1][y] == 0) { batterylife--; step.push(path[x-1][y]);map[x-1][y] = 1;x--;}
                        else if(map[x][y-1] == 0) {batterylife--; step.push(path[x][y-1]);map[x][y-1] = 1; y--;}
                        else {cout<<"DeadEnd"<<endl; break;}
                        
                        s = path[x][y]->weight;
                        //cout<<"( "<<path[x][y]->row<<","<<path[x][y]->col<<" )"<<endl;
                        // PrintMap();
                        // cout<<endl;
                    }
                    //cout<<"Recharge"<<endl;
                    NodeItem* temps =  path[x][y];
                    pushSteps_toRoot(temps,root);
                    batterylife = battery;
                }
            }
        }
    
    cout<<"All Cleaned"<<endl;
}

void Robot::outStep(ofstream& outFile) {
    while(!step.empty())
    {
        outFile<<step.front()->row<<" "<<step.front()->col<<" "<<endl;
        step.pop();
    }
}

void Robot::PrintStep() {
    while(!step.empty())
    {
        cout<<step.front()->row<<" "<<step.front()->col<<" "<<endl;
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
    //  if(flag == 851585) {
    //     cout<<"\nLarge"<<endl;
    //     robot.Move_Large();
      
    //  }
    //  else{
         robot.Move();
    //  }
    cout<<"num_of_node: "<<flag<<endl;
    ofstream outFile("final.path", ios::out); 
    outFile<<robot.steps()<<endl;
    // robot.PrintMap();
    //robot.PrintStep();
     robot.outStep(outFile);
}


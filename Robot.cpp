#include <iostream>
#include <fstream>
#include <stack>
#include <queue>
#include <algorithm>
#include <cmath>
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
        int** map;
        bool** mapSP; // original bool map
        bool** mapcopy; // use in BFS
        stack<NodeItem*> track;
        queue<NodeItem*> step; 
    public:
        //constructor; initinalize map, battery life, num_node and root node 
        Robot(int r, int c, int b, char** input):row(r),col(c),battery(b){
            // construct map
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
                        track.push(root);
                    }
                    else if(input[i][j]=='1'){
                        map[i][j] = 2;
                        mapSP[i][j] = false;
                    }
                    else{
                        map[i][j] = 0;
                        num_node++;
                        mapSP[i][j] = true;
                    }
                }
            }
            cout<<"num_node: "<<num_node<<"\nRoot: ("<<track.top()->row<<", "<<track.top()->col<<")"<<" track.size(): "<<track.size()<<endl;
            /*
            1 1 1 1         2 2 2 2 
            1 0 0 1   ===>  2 0 0 2  ===> num_node = 4, not including root
            1 1 0 1         2 2 0 2
            1 0 R 1         2 0 1 2   
            */
        }
        void mapCopyInit();
        bool AllClean();
        void Move();
        void PrintStep();
        void PrintMap();
        void PrintMapCopy();
        int AstarPath(queue<NodeItem*>& step);
        vector<vector<NodeItem*> > ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to);
        bool NodeFinish(NodeItem* top);
};

 bool Robot::NodeFinish(NodeItem* top) {
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
/*
FFFFFF
FTTTTF
FSTTTD
FFFFFF
*/
vector<vector<NodeItem*> > Robot::ShortestPath_to_R_BFS(NodeItem* from, NodeItem* to) {
    // NodeItem** path = new NodeItem*[row];
    // for(int i = 0; i < row; i++) {
    //     path[i] = new NodeItem[col];
    // }
    // for(int i = 0; i < row; i++) {
    //     for(int j = 0; j < col; j++) {
    //         path[i][j] = NULL;
    //     }
    // }
    vector<vector<NodeItem*> > path;
    path.resize(row);
    for(int i = 0; i < row; i++) { path[i].resize(col); }
    
    mapCopyInit();
    queue<NodeItem*> bfs;
    stack<NodeItem*> s;
    cout<<"Start:"<<endl;
    PrintMapCopy();
    bfs.push(from);
    while(!bfs.empty()) {
        NodeItem* current = bfs.front();
        int r = current->row;
        int c = current->col;
        mapcopy[r][c] = false;
        bfs.pop();
        
        // case 0 : from == to
        if(r == to->row && c == to->col) break;
        
        int weightUp = abs(r-1 - root->row)+ abs(c - root->col);
        int weightDown = abs(r+1 - root->row) + abs(c - root->col);
        int weightLeft = abs(r - root->row) + abs(c-1 - root->col);
        int weightRight = abs(r - root->row) + abs(c+1 - root->col);

        if(r-1 >= 0 && mapcopy[r-1][c] == true ) { NodeItem* up = new NodeItem(r-1, c, weightUp); mapcopy[r-1][c] =false; up->parent = current; path[r-1][c] = up; bfs.push(up); if(r-1 == to->row && c == to->col) break;}
        if(r+1 <= row-1 && mapcopy[r+1][c] == true) { NodeItem* down = new NodeItem(r+1, c, weightDown); mapcopy[r+1][c] = false; down->parent = current; path[r+1][c] = down; bfs.push(down); if(r+1 == to->row && c == to->col) break;} 
        if(c-1 >= 0 && mapcopy[r][c-1] == true ) { NodeItem* left = new NodeItem(r, c-1, weightLeft); mapcopy[r][c-1] = false; left->parent = current; path[r][c-1] = left; bfs.push(left); if(r == to->row && c-1 == to->col) break;}
        if(c+1 <= col-1 && mapcopy[r][c+1] == true) { NodeItem* right = new NodeItem(r, c+1, weightRight); mapcopy[r][c+1] = false; right->parent = current; path[r][c+1] = right; bfs.push(right); if(r == to->row && c+1 == to->col) break;}
    }
    cout<<"To:"<<endl;
    PrintMapCopy();
    cout<<endl;
    //NodeItem* temp = path[to->row][to->col];
    // int counts = 0;
    // while(temp != from) {
    //     cout<<"Parent: "<<"[ "<< temp->row<<", "<<temp->col<<" ]"<<endl;
    //     temp = temp->parent;
    //     counts++;
    // }
    // cout<<"count: "<<counts<<endl;
    return path;
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

void Robot::Move() {
    int count = 0;
    stack<NodeItem*> s1;
    stack<NodeItem*> s2;
    stack<NodeItem*> s3;
    queue<NodeItem*> rushback;
    while(!AllClean()) // while(!track.empty())
    {
        int r = track.top()->row;
        int c = track.top()->col;
        int w = track.top()->weight;
        NodeItem* newnode;
        // Moving up
        if(r-1 >= 0 && map[r-1][c] == 0 ) {
            //visiting
            newnode = new NodeItem(r-1, c, w+1);
            map[r-1][c] = 1;
            track.push(newnode);
            step.push(newnode);
            count++;
        }
        // Moving down
        else if(r+1 <= row-1 && map[r+1][c] == 0 ) {
            //visiting
            newnode = new NodeItem(r+1, c, w+1);
            map[r+1][c] = 1;
            track.push(newnode);
            step.push(newnode);
            count++;
        }
        // Moving Left
        else if(c-1 >= 0 && map[r][c-1] == 0 ) {
            //visiting
            newnode = new NodeItem(r, c-1, w+1);
            map[r][c-1] = 1;
            track.push(newnode);
            step.push(newnode);
            count++;    
        }
        // Moving Right
        else if(c+1 <= col-1 && map[r][c+1] == 0 ) {
            //visiting
            newnode = new NodeItem(r, c+1, w+1);
            map[r][c+1] = 1;
            track.push(newnode);
            step.push(newnode);
            count++;
        }
        // Dead-End: 1. Search for the node that has unvisited adjancey node(s)
        else {
            NodeItem* from = track.top();
            cout<<"From the dead-end node: "<<"[ "<< from->row<<", "<<from->col<<" ]"<<endl;
            
            while(NodeFinish(track.top())) {
                map[track.top()->row][track.top()->col] = 2;
                track.pop();
            }
            NodeItem* to = track.top();
            cout<<"Back to the node with unvisited adjancey nodes: "<<"[ "<< to->row<<", "<<to->col<<" ]"<<endl;
            cout<<endl;
            
            //need fix
            vector<vector<NodeItem*> > destination = ShortestPath_to_R_BFS(from, to);
            //
            cout<<endl;
            int counts = 0;
            NodeItem* temp = destination[to->row][to->col];
            while(temp != from) {
                //cout<<"Parent: "<<"[ "<< temp->row<<", "<<temp->col<<" ]"<<endl;
                s1.push(temp);
                temp = temp->parent;
                counts++;
            }
            
            // if (to == root) then goRoot->parent == NULLs
            //
            vector<vector<NodeItem*> >goRoot = ShortestPath_to_R_BFS(to, root);
            //need fix
            int counting = 0;
            NodeItem* temps = goRoot[root->row][root->col];
            while(temps != to) {
                s2.push(temp);
                temps = temps->parent;
                counting++;
            }
            while(!s2.empty()) {
                    rushback.push(s2.top());
                    //rushback.push(s1.top());
                    s2.pop();
                }
            // it is safe to go 
            if(counts+step.size()+counting <= battery) {
                while(!s1.empty()) {
                    step.push(s1.top());
                    //rushback.push(s1.top());
                    s1.pop();
                }
            }
            //  need to recharge first
            else {
                cout<<"\nRecharge: "<<(counts+step.size()+counting)<<" > "<<battery<<endl;
                /*Recharge*/
                //need fix
                vector<vector<NodeItem*> >goCharge = ShortestPath_to_R_BFS(from, root);
                //
                NodeItem* tempss = goCharge[root->row][root->col];
                while(tempss != from) {
                    s3.push(tempss);
                    tempss = tempss->parent;
                }
                while(!s3.empty()) {
                    step.push(s3.top());
                    s3.pop();
                }
                /*Rushback*/
                //need fix
                while(!rushback.empty()) {
                    step.push(rushback.front());
                    rushback.pop();
                }

            }

            //count += counts;
            //now shortestpath_goRoot(from, to) (need to recharge?) then same route but reverse shortestpath_goRoot(to, from)
            //calculate battery life
            //Recharge()
        }
        
        PrintMap();
        cout<<"step: "<<step.size()<<endl;
        cout<<endl;      
    }
    /*
    After all clean if (stack.top() != root) then shortestPath back to root
    */
   cout<<"Timetogohome: "<<"[ "<< track.top()->row<<", "<<track.top()->col<<" ]"<<endl;
   vector<vector<NodeItem*> >Home = ShortestPath_to_R_BFS(track.top(), root);
   NodeItem* temp = Home[root->row][root->col];
    while(temp != track.top()) {
        //cout<<"Parent: "<<"[ "<< temp->row<<", "<<temp->col<<" ]"<<endl;
        s3.push(temp);
        temp = temp->parent;
    }
    while(!s3.empty()) {
        step.push(s3.top());
        s3.pop();
    }

}

void Robot::PrintStep() {
    cout<<step.size()<<endl;
    while(!step.empty())
    {
        cout<<"[ "<< step.front()->row<<", "<<step.front()->col<<" ]"<<endl;
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

void Robot::PrintMapCopy() {
    for(int i = 0; i < row; i++)
    {
        for(int j = 0; j < col;j++)
        {
            cout<<mapcopy[i][j];
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
    robot.PrintMap();
    cout<<endl;
    robot.Move();
    robot.PrintStep();
}

/*
        // choose the farthest node
        int weightUp = abs(r-1 - root->row)+ abs(c - root->col);
        int weightDown = abs(r+1 - root->row) + abs(c - root->col);
        int weightLeft = abs(r - root->row) + abs(c-1 - root->col);
        int weightRight = abs(r - root->row) + abs(c+1 - root->col);
    
        int* weightArray = new int[4];
        weightArray[0] = weightUp; weightArray[1] = weightDown; weightArray[2] = weightLeft; weightArray[3] = weightRight;
        bool* Go = new bool[4];
        for(int i = 0; i < 4; i++) {
             int farthest = weightArray[i];
             Go[i] = false;
             cout<< farthest<<" ";
        }
        cout<<endl;
        if(r-1 >= 0 && map[r-1][c] == 0 ) Go[0] = true;
        if(r+1 <= row-1 && map[r+1][c] == 0 ) Go[1] = true;
        if(c-1 >= 0 && map[r][c-1] == 0 ) Go[2] = true;
        if(c+1 >= 0 && map[r][c+1] == 0 ) Go[3] = true;
        
        int farthest = 0;
        for(int i = 0; i < 4; i++) {
            if (farthest < weightArray[i] && Go[i]) { 
                farthest = weightArray[i];      
            }
        }
        
        // Moving up
        if(farthest == weightUp && Go[0]) {
            //visiting
            newnode = new NodeItem(r-1, c, w+1);
            map[r-1][c] = 1;
            track.push(newnode);
        }
        // Moving down
        else if(farthest == weightDown && Go[1]) {
            //visiting
            newnode = new NodeItem(r+1, c, w+1);
            map[r+1][c] = 1;
            track.push(newnode);
        }
        // Moving Left
        else if(farthest == weightLeft && Go[2]) {
            //visiting
            newnode = new NodeItem(r, c-1, w+1);
            map[r][c-1] = 1;
            track.push(newnode);    
        }
        // Moving Right
        else if(farthest == weightRight && Go[3]) {
            //visiting
            newnode = new NodeItem(r, c+1, w+1);
            map[r][c+1] = 1;
            track.push(newnode);
        }
        else {
            map[r][c] = 2;
            track.pop();
        }
 */
        
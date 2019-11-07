template <class T>
struct Node{
    T data;
    Node* next_node;
    Node(T data = 0):data(data){}
    Node(T data, Node* next):data(data),next_node(next){}
};
template <class T>
class queue{
    private:
        struct Node{
            T data;
            Node* next_node;
            Node(T data = 0):data(data){}
            Node(T data, Node* next):data(data),next_node(next){}
        };
        Node* frontNode;
        Node* backNode;
        int Size;
    public:
        queue():Size(0),frontNode(0), backNode(0){}
        void push(T);
        int size();
        T front();
        bool empty();
        void pop();
};

template <class T> 
inline int queue<T>::size(){
    return this->Size;
}
template <class T> 
inline bool queue<T>::empty(){
    if(this->size() == 0){
        return true;
    }
    return false;
}
template <class T> 
inline void queue<T>::push(T n){
    if(empty()){
        frontNode = new Node(n);
        backNode = frontNode;
        this->Size++;
        return;
    }
    Node* newnode = new Node(n);//point to the current top address
    backNode->next_node = newnode; //Update the top address to newnode
    backNode = newnode;
    this->Size++;
}
template <class T> 
inline void queue<T>::pop() {
    if(empty())return;
    Node* deletenode = frontNode;
    frontNode = frontNode->next_node;
    delete deletenode;
    deletenode = 0;
    Size--;
}
template <class T> 
inline T queue<T>::front(){
    if(empty()){
        return 0;
    }
    return frontNode->data;
}
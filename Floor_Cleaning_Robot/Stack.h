template <class T>
class stack{
    private:
        struct Node{
            T data;
            Node* next_node;
            Node(T data = 0):data(data){}
            Node(T data, Node* next):data(data),next_node(next){}
        };
        Node* topNode;
        int Size;
    public:
        stack(): Size(0),topNode(0){}
        void push(T element);
        int size();
        T top();
        bool empty();
        void pop();
};

template <class T> 
inline int stack<T>::size(){
    return this->Size;
}
template <class T> 
inline bool stack<T>::empty(){
    if(this->size() == 0){
        return true;
    }
    return false;
}
template <class T> 
inline void stack<T>::push(T n){
    if(empty()){
        Node* newnode = new Node(n,0);
        this->topNode = newnode;
        this->Size++;
        return;
    }
    Node* newnode = new Node(n,topNode);//point to the current top address
    topNode = newnode; //Update the top address to newnode
    this->Size++;
}
template <class T> 
inline void stack<T>::pop() {
    if(empty()) return;
    Node* deletenode = topNode;
    topNode = topNode->next_node;
    delete deletenode;
    deletenode = 0;
    Size--;
}
template <class T> 
inline T stack<T>::top(){
    if(empty()){
        return 0;
    }
    return topNode->data;
}
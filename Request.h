#ifndef REQUEST_H
#define REQUEST_H

#include "User.h" // Assuming User is correctly defined elsewhere
#include <string>

class Request {
public:
    enum RequestState {
        RequestGenerated,
        RequestSent,
        RequestAgree,
        RequestRefused
    };

    User* fromUser;
    User* toUser;
    RequestState currentState;

public:
    Request(User* from, User* to, Request::RequestState);
    Request();
    Request(const Request &other){
        fromUser = other.fromUser;
        toUser = other.toUser;
        currentState = other.currentState;
    }

    // Getters
    User* getFromUser() const { return fromUser; }
    User* getToUser() const { return toUser; }
    RequestState getCurrentState() const { return currentState; }

    // Setters
    void setFromUser(User* from) { fromUser = from; }
    void setToUser(User* to) { toUser = to; }
    void setCurrentState(RequestState state) { currentState = state; }

    static Request* isMatchUserFromUser(const std::vector<Request*>& requestList, User* matchUser, User* requestingUser);
};

#endif // REQUEST_H

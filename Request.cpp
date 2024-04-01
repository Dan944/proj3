#include "Request.h"

Request::Request(User* from, User* to, Request::RequestState) : fromUser(from), toUser(to), currentState(RequestGenerated) {}
Request::Request() : fromUser(nullptr), toUser(nullptr), currentState(RequestGenerated) {}

Request* Request::isMatchUserFromUser(const std::vector<Request*>& requestList, User* matchUser, User* requestingUser) {
    for (const auto& request : requestList) {
        if (request->fromUser == matchUser) {
            if (request->toUser == requestingUser) {
                return request; // Found a request with matchUser as fromUser
            }
        }
    }
    return nullptr; // No matching request found
}

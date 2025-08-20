#pragma once

#include <string>
#include <set>
#include <map>
#include <vector>

class Client;

class Channel
{
    private:
        std::string _name;
        std::string _topic;
        std::string _password;
        std::set<int> _members;          // FDs des clients membres
        std::set<int> _operators;        // FDs des opérateurs
        std::set<int> _invited;          // FDs des clients invités (mode +i)
        
        // Modes du canal
        bool _inviteOnly;                // Mode +i
        bool _topicRestricted;           // Mode +t
        bool _hasPassword;               // Mode +k
        int _userLimit;                  // Mode +l (0 = pas de limite)

    public:
        Channel(const std::string &name);
        ~Channel();

        // Getters
        const std::string &getName() const;
        const std::string &getTopic() const;
        const std::string &getPassword() const;
        const std::set<int> &getMembers() const;
        const std::set<int> &getOperators() const;
        bool isInviteOnly() const;
        bool isTopicRestricted() const;
        bool hasPassword() const;
        int getUserLimit() const;
        size_t getMemberCount() const;

        // Setters
        void setTopic(const std::string &topic);
        void setPassword(const std::string &password);
        void setInviteOnly(bool inviteOnly);
        void setTopicRestricted(bool topicRestricted);
        void setUserLimit(int limit);

        // Gestion des membres
        bool addMember(int clientFd);
        bool removeMember(int clientFd);
        bool isMember(int clientFd) const;
        
        // Gestion des opérateurs
        bool addOperator(int clientFd);
        bool removeOperator(int clientFd);
        bool isOperator(int clientFd) const;
        
        // Gestion des invitations
        bool addInvited(int clientFd);
        bool removeInvited(int clientFd);
        bool isInvited(int clientFd) const;
        
        // Vérifications
        bool canJoin(int clientFd, const std::string &password = "") const;
        bool isEmpty() const;
        
        // Utilitaires
        std::string getModeString() const;
        std::vector<int> getAllMembers() const;
};
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>
#include <chrono>
#include <memory>

namespace gyatt {

class CommentThread {
public:
    struct Comment {
        std::string id;
        std::string filepath;
        size_t lineNumber;
        std::string author;
        std::string message;
        std::chrono::system_clock::time_point timestamp;
        bool resolved;
        std::vector<Comment> replies;
    };

    CommentThread(const std::string& repoPath);
    
    // Comment operations
    bool addComment(const std::string& filepath, int lineNumber, 
                   const std::string& message, const std::string& author);
    bool replyToComment(const std::string& commentId, const std::string& message,
                       const std::string& author);
    bool resolveComment(const std::string& commentId);
    
    // Thread management
    std::vector<Comment> getCommentsForFile(const std::string& filepath);
    std::vector<Comment> getAllComments();
    void showCommentsForFile(const std::string& filepath);
    void showAllComments();
    
private:
    std::string repoPath;
    std::string commentsDir;
    std::string commentsFile;
    std::map<std::string, Comment> comments;
    
    void showComment(const Comment& comment, bool showFile = true);
    std::string generateCommentId();
    bool saveComments();
    bool loadComments();
};

class StickyNotes {
public:
    struct StickyNote {
        std::string id;
        std::string content;
        std::string category;
        std::string author;
        std::string filepath;
        int lineNumber;
        int priority;
        std::chrono::system_clock::time_point timestamp;
        bool pinned;
    };

    StickyNotes(const std::string& repoPath);
    
    // Note operations
    bool addNote(const std::string& content, const std::string& category = "todo",
                 const std::string& filepath = "", int lineNumber = 0);
    bool removeNote(const std::string& noteId);
    bool pinNote(const std::string& noteId);
    bool updateNote(const std::string& noteId, const std::string& newContent);
    
    // Note management
    std::vector<StickyNote> getNotesByCategory(const std::string& category);
    std::vector<StickyNote> getAllNotes();
    void showAllNotes();
    void showNotesByCategory(const std::string& category);
    void showNotesByPriority(bool pinned);
    
private:
    std::string repoPath;
    std::string notesDir;
    std::string notesFile;
    std::vector<StickyNote> notes;
    
    void showNote(const StickyNote& note);
    std::string generateNoteId();
    bool saveNotes();
    bool loadNotes();
};

class LabelSystem {
public:
    struct LabelDefinition {
        std::string name;
        std::string color;
        std::string description;
        std::chrono::system_clock::time_point created;
    };

    LabelSystem(const std::string& repoPath);
    
    // Label operations
    bool addLabel(const std::string& filepath, const std::string& labelName, 
                 const std::string& description = "");
    bool removeLabel(const std::string& filepath, const std::string& labelName);
    bool createLabelDefinition(const std::string& name, const std::string& color,
                              const std::string& description);
    
    // File labeling
    bool addFileLabel(const std::string& filepath, const std::string& label);
    bool removeFileLabel(const std::string& filepath, const std::string& label);
    std::vector<std::string> getFileLabels(const std::string& filepath);
    std::set<std::string> getLabelsForFile(const std::string& filepath);
    
    // Label management
    std::vector<std::string> getAllLabels();
    std::vector<std::string> getFilesWithLabel(const std::string& label);
    std::map<std::string, LabelDefinition> getAllLabelDefinitions();
    bool renameLabel(const std::string& oldLabel, const std::string& newLabel);
    bool deleteLabel(const std::string& label);
    
    // Display methods
    void showAllLabels();
    void showFileLabels(const std::string& filepath);
    void showLabelFiles(const std::string& labelName);
    
    // Querying
    std::vector<std::string> queryChangesByLabel(const std::string& label);
    void showLabelLog(const std::string& label, int limit = -1);
    void showLabelSummary();
    
    // Built-in labels
    bool autoLabelCore();
    bool autoLabelTemp();
    bool autoLabelLegacy();
    bool autoLabelTests();
    
private:
    std::string repoPath;
    std::string labelsFile;
    std::map<std::string, std::vector<std::string>> fileLabelMap;
    std::map<std::string, std::set<std::string>> fileLabels;
    std::map<std::string, LabelDefinition> labelDefinitions;
    
    bool saveLabels();
    bool loadLabels();
    bool isLegacyFile(const std::string& filepath);
    bool isTempFile(const std::string& filepath);
    bool isCoreFile(const std::string& filepath);
};

} // namespace gyatt

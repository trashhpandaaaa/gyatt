#include "../include/enhanced_features.h"
#include "../include/utils.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <regex>
#include <sstream>
#include <ctime>

namespace gyatt {

// CommentThread Implementation

CommentThread::CommentThread(const std::string& repoPath) 
    : repoPath(repoPath), commentsFile(repoPath + "/.gyatt/comments.json") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadComments();
}

bool CommentThread::addComment(const std::string& filepath, int lineNumber, 
                               const std::string& message, const std::string& author) {
    Comment comment;
    comment.id = generateCommentId();
    comment.filepath = filepath;
    comment.lineNumber = lineNumber;
    comment.message = message;
    comment.author = author;
    comment.timestamp = std::chrono::system_clock::now();
    comment.resolved = false;
    
    comments[comment.id] = comment;
    saveComments();
    
    std::cout << "💬 Added comment to " << filepath << ":" << lineNumber << std::endl;
    std::cout << "📝 \"" << message << "\" - " << author << std::endl;
    
    return true;
}

bool CommentThread::replyToComment(const std::string& commentId, const std::string& message, 
                                   const std::string& author) {
    auto it = comments.find(commentId);
    if (it != comments.end()) {
        Comment reply;
        reply.id = generateCommentId();
        reply.message = message;
        reply.author = author;
        reply.timestamp = std::chrono::system_clock::now();
        reply.resolved = false;
        
        it->second.replies.push_back(reply);
        saveComments();
        
        std::cout << "↳ 💬 Reply added to comment " << commentId << std::endl;
        std::cout << "📝 \"" << message << "\" - " << author << std::endl;
        return true;
    }
    
    std::cerr << "❌ Comment not found: " << commentId << std::endl;
    return false;
}

bool CommentThread::resolveComment(const std::string& commentId) {
    auto it = comments.find(commentId);
    if (it != comments.end()) {
        it->second.resolved = true;
        saveComments();
        
        std::cout << "✅ Resolved comment: " << commentId << std::endl;
        return true;
    }
    
    std::cout << "❌ Comment not found: " << commentId << std::endl;
    return false;
}

std::vector<CommentThread::Comment> CommentThread::getCommentsForFile(const std::string& filepath) {
    std::vector<Comment> fileComments;
    
    for (const auto& [id, comment] : comments) {
        if (comment.filepath == filepath && !comment.resolved) {
            fileComments.push_back(comment);
        }
    }
    
    return fileComments;
}

std::vector<CommentThread::Comment> CommentThread::getAllComments() {
    std::vector<Comment> allComments;
    for (const auto& [id, comment] : comments) {
        allComments.push_back(comment);
    }
    return allComments;
}

void CommentThread::showCommentsForFile(const std::string& filepath) {
    auto fileComments = getCommentsForFile(filepath);
    
    if (fileComments.empty()) {
        std::cout << "📝 No active comments for " << filepath << std::endl;
        return;
    }
    
    std::cout << "\n💬 Comments for " << filepath << "\n";
    std::cout << "═══════════════════════════════════════\n";
    
    for (const auto& comment : fileComments) {
        showComment(comment);
        std::cout << "───────────────────────────────────────\n";
    }
}

void CommentThread::showAllComments() {
    if (comments.empty()) {
        std::cout << "📝 No comments in repository" << std::endl;
        return;
    }
    
    std::cout << "\n💬 All Comments\n";
    std::cout << "═══════════════════════════════════════\n";
    
    // Group by file
    std::map<std::string, std::vector<Comment>> commentsByFile;
    for (const auto& [id, comment] : comments) {
        if (!comment.resolved) {
            commentsByFile[comment.filepath].push_back(comment);
        }
    }
    
    for (const auto& [filepath, fileComments] : commentsByFile) {
        std::cout << "\n📁 " << filepath << "\n";
        for (const auto& comment : fileComments) {
            showComment(comment, false);
        }
    }
}

void CommentThread::showComment(const Comment& comment, bool showFile) {
    auto timeStr = Utils::formatTime(comment.timestamp);
    
    if (showFile) {
        std::cout << "📁 " << comment.filepath << ":" << comment.lineNumber << std::endl;
    } else {
        std::cout << "  Line " << comment.lineNumber << ": ";
    }
    
    std::cout << "💬 " << comment.message << std::endl;
    std::cout << "👤 " << comment.author << " • " << timeStr;
    
    if (comment.resolved) {
        std::cout << " • ✅ Resolved";
    }
    std::cout << " • ID: " << comment.id << std::endl;
    
    // Show replies
    for (const auto& reply : comment.replies) {
        auto replyTimeStr = Utils::formatTime(reply.timestamp);
        std::cout << "  ↳ " << reply.message << std::endl;
        std::cout << "    👤 " << reply.author << " • " << replyTimeStr << std::endl;
    }
}

std::string CommentThread::generateCommentId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return "comment_" + std::to_string(timestamp) + "_" + std::to_string(rand() % 1000);
}

bool CommentThread::saveComments() {
    std::ofstream file(commentsFile);
    if (!file.is_open()) return false;
    
    file << "{\n  \"comments\": [\n";
    
    size_t count = 0;
    for (const auto& [id, comment] : comments) {
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            comment.timestamp.time_since_epoch()).count();
        
        file << "    {\n";
        file << "      \"id\": \"" << comment.id << "\",\n";
        file << "      \"filepath\": \"" << comment.filepath << "\",\n";
        file << "      \"lineNumber\": " << comment.lineNumber << ",\n";
        file << "      \"message\": \"" << comment.message << "\",\n";
        file << "      \"author\": \"" << comment.author << "\",\n";
        file << "      \"timestamp\": " << timestamp << ",\n";
        file << "      \"resolved\": " << (comment.resolved ? "true" : "false");
        
        if (!comment.replies.empty()) {
            file << ",\n      \"replies\": [\n";
            for (size_t j = 0; j < comment.replies.size(); ++j) {
                const auto& reply = comment.replies[j];
                auto replyTimestamp = std::chrono::duration_cast<std::chrono::seconds>(
                    reply.timestamp.time_since_epoch()).count();
                
                file << "        {\n";
                file << "          \"id\": \"" << reply.id << "\",\n";
                file << "          \"message\": \"" << reply.message << "\",\n";
                file << "          \"author\": \"" << reply.author << "\",\n";
                file << "          \"timestamp\": " << replyTimestamp << "\n";
                file << "        }";
                
                if (j < comment.replies.size() - 1) file << ",";
                file << "\n";
            }
            file << "      ]";
        }
        
        file << "\n    }";
        if (count < comments.size() - 1) file << ",";
        file << "\n";
        count++;
    }
    
    file << "  ]\n}\n";
    return true;
}

bool CommentThread::loadComments() {
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    std::ifstream file(commentsFile);
    if (!file.is_open()) return false;
    
    // For now, just create empty structure
    comments.clear();
    return true;
}

// StickyNotes Implementation

StickyNotes::StickyNotes(const std::string& repoPath) 
    : repoPath(repoPath), notesFile(repoPath + "/.gyatt/sticky_notes.json") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadNotes();
}

bool StickyNotes::addNote(const std::string& content, const std::string& category, 
                          const std::string& filepath, int lineNumber) {
    StickyNote note;
    note.id = generateNoteId();
    note.content = content;
    note.category = category;
    note.author = "user"; // Default author
    note.filepath = filepath;
    note.lineNumber = lineNumber;
    note.priority = 1; // Default priority
    note.timestamp = std::chrono::system_clock::now();
    note.pinned = false;
    
    notes.push_back(note);
    saveNotes();
    
    std::cout << "📌 Added sticky note (" << category << "): " << content << std::endl;
    return true;
}

bool StickyNotes::removeNote(const std::string& noteId) {
    auto it = std::find_if(notes.begin(), notes.end(),
        [&noteId](const StickyNote& note) { return note.id == noteId; });
    
    if (it != notes.end()) {
        std::cout << "🗑️  Removed note: " << it->content << std::endl;
        notes.erase(it);
        saveNotes();
        return true;
    }
    
    std::cout << "❌ Note not found: " << noteId << std::endl;
    return false;
}

bool StickyNotes::pinNote(const std::string& noteId) {
    for (auto& note : notes) {
        if (note.id == noteId) {
            note.pinned = true;
            saveNotes();
            std::cout << "📌 Pinned note: " << note.content << std::endl;
            return true;
        }
    }
    
    std::cout << "❌ Note not found: " << noteId << std::endl;
    return false;
}

bool StickyNotes::updateNote(const std::string& noteId, const std::string& newContent) {
    for (auto& note : notes) {
        if (note.id == noteId) {
            note.content = newContent;
            note.timestamp = std::chrono::system_clock::now();
            saveNotes();
            std::cout << "✏️  Updated note: " << newContent << std::endl;
            return true;
        }
    }
    
    std::cout << "❌ Note not found: " << noteId << std::endl;
    return false;
}

std::vector<StickyNotes::StickyNote> StickyNotes::getNotesByCategory(const std::string& category) {
    std::vector<StickyNote> categoryNotes;
    
    for (const auto& note : notes) {
        if (note.category == category) {
            categoryNotes.push_back(note);
        }
    }
    
    return categoryNotes;
}

std::vector<StickyNotes::StickyNote> StickyNotes::getAllNotes() {
    return notes;
}

void StickyNotes::showAllNotes() {
    if (notes.empty()) {
        std::cout << "📝 No sticky notes" << std::endl;
        return;
    }
    
    std::cout << "\n📌 Sticky Notes\n";
    std::cout << "═══════════════════════════════════════\n";
    
    // Show pinned notes first
    showNotesByPriority(true);
    showNotesByPriority(false);
}

void StickyNotes::showNotesByCategory(const std::string& category) {
    auto categoryNotes = getNotesByCategory(category);
    
    if (categoryNotes.empty()) {
        std::cout << "📝 No notes in category: " << category << std::endl;
        return;
    }
    
    std::cout << "\n📌 " << category << " Notes\n";
    std::cout << "═══════════════════════════════════════\n";
    
    for (const auto& note : categoryNotes) {
        showNote(note);
    }
}

void StickyNotes::showNote(const StickyNote& note) {
    auto timeStr = Utils::formatTime(note.timestamp);
    
    if (note.pinned) {
        std::cout << "📌 ";
    } else {
        std::cout << "📝 ";
    }
    
    std::cout << note.content << std::endl;
    std::cout << "   📂 " << note.category << " • 👤 " << note.author 
              << " • " << timeStr << " • Priority: " << note.priority 
              << " • ID: " << note.id << std::endl;
    std::cout << "───────────────────────────────────────\n";
}

void StickyNotes::showNotesByPriority(bool pinned) {
    std::vector<StickyNote> filteredNotes;
    
    for (const auto& note : notes) {
        if (note.pinned == pinned) {
            filteredNotes.push_back(note);
        }
    }
    
    // Sort by priority (higher first)
    std::sort(filteredNotes.begin(), filteredNotes.end(),
        [](const StickyNote& a, const StickyNote& b) {
            return a.priority > b.priority;
        });
    
    if (pinned && !filteredNotes.empty()) {
        std::cout << "📌 PINNED NOTES\n";
    }
    
    for (const auto& note : filteredNotes) {
        showNote(note);
    }
}

std::string StickyNotes::generateNoteId() {
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    return "note_" + std::to_string(timestamp) + "_" + std::to_string(rand() % 1000);
}

bool StickyNotes::saveNotes() {
    std::ofstream file(notesFile);
    if (!file.is_open()) return false;
    
    file << "{\n  \"notes\": [\n";
    
    for (size_t i = 0; i < notes.size(); ++i) {
        const auto& note = notes[i];
        auto timestamp = std::chrono::duration_cast<std::chrono::seconds>(
            note.timestamp.time_since_epoch()).count();
        
        file << "    {\n";
        file << "      \"id\": \"" << note.id << "\",\n";
        file << "      \"content\": \"" << note.content << "\",\n";
        file << "      \"category\": \"" << note.category << "\",\n";
        file << "      \"author\": \"" << note.author << "\",\n";
        file << "      \"priority\": " << note.priority << ",\n";
        file << "      \"timestamp\": " << timestamp << ",\n";
        file << "      \"pinned\": " << (note.pinned ? "true" : "false") << "\n";
        file << "    }";
        
        if (i < notes.size() - 1) file << ",";
        file << "\n";
    }
    
    file << "  ]\n}\n";
    return true;
}

bool StickyNotes::loadNotes() {
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    std::ifstream file(notesFile);
    if (!file.is_open()) return false;
    
    // For now, just create empty structure
    notes.clear();
    return true;
}

// LabelSystem Implementation

LabelSystem::LabelSystem(const std::string& repoPath) 
    : repoPath(repoPath), labelsFile(repoPath + "/.gyatt/labels.json") {
    std::filesystem::create_directories(repoPath + "/.gyatt");
    loadLabels();
}

bool LabelSystem::addLabel(const std::string& filepath, const std::string& labelName, 
                           const std::string& description) {
    // Create label if it doesn't exist
    if (labelDefinitions.find(labelName) == labelDefinitions.end()) {
        LabelDefinition def;
        def.name = labelName;
        def.color = "blue"; // Default color
        def.description = description.empty() ? "Auto-created label" : description;
        def.created = std::chrono::system_clock::now();
        labelDefinitions[labelName] = def;
    }
    
    // Add file to label
    fileLabels[filepath].insert(labelName);
    saveLabels();
    
    std::cout << "🏷️  Added label '" << labelName << "' to " << filepath << std::endl;
    return true;
}

bool LabelSystem::removeLabel(const std::string& filepath, const std::string& labelName) {
    auto fileIt = fileLabels.find(filepath);
    if (fileIt != fileLabels.end()) {
        fileIt->second.erase(labelName);
        if (fileIt->second.empty()) {
            fileLabels.erase(fileIt);
        }
        saveLabels();
        
        std::cout << "🗑️  Removed label '" << labelName << "' from " << filepath << std::endl;
        return true;
    }
    
    std::cout << "❌ No label '" << labelName << "' found on " << filepath << std::endl;
    return false;
}

bool LabelSystem::createLabelDefinition(const std::string& name, const std::string& color, 
                                         const std::string& description) {
    LabelDefinition def;
    def.name = name;
    def.color = color;
    def.description = description;
    def.created = std::chrono::system_clock::now();
    
    labelDefinitions[name] = def;
    saveLabels();
    
    std::cout << "🏷️  Created label definition: " << name << " (" << color << ")" << std::endl;
    return true;
}

bool LabelSystem::addFileLabel(const std::string& filepath, const std::string& labelName) {
    return addLabel(filepath, labelName, "");
}

std::set<std::string> LabelSystem::getLabelsForFile(const std::string& filepath) {
    auto it = fileLabels.find(filepath);
    if (it != fileLabels.end()) {
        return it->second;
    }
    return {};
}

std::vector<std::string> LabelSystem::getAllLabels() {
    std::vector<std::string> labels;
    for (const auto& [name, def] : labelDefinitions) {
        labels.push_back(name);
    }
    return labels;
}

std::vector<std::string> LabelSystem::getFilesWithLabel(const std::string& labelName) {
    std::vector<std::string> files;
    
    for (const auto& [filepath, labels] : fileLabels) {
        if (labels.find(labelName) != labels.end()) {
            files.push_back(filepath);
        }
    }
    
    return files;
}

std::map<std::string, LabelSystem::LabelDefinition> LabelSystem::getAllLabelDefinitions() {
    return labelDefinitions;
}

void LabelSystem::showAllLabels() {
    if (labelDefinitions.empty()) {
        std::cout << "🏷️  No labels defined" << std::endl;
        return;
    }
    
    std::cout << "\n🏷️  Label Definitions\n";
    std::cout << "═══════════════════════════════════════\n";
    
    for (const auto& [name, def] : labelDefinitions) {
        auto files = getFilesWithLabel(name);
        std::cout << "🏷️  " << name << " (" << def.color << ")\n";
        std::cout << "   📝 " << def.description << "\n";
        std::cout << "   📁 " << files.size() << " files tagged\n";
        std::cout << "───────────────────────────────────────\n";
    }
}

void LabelSystem::showFileLabels(const std::string& filepath) {
    auto labels = getLabelsForFile(filepath);
    
    if (labels.empty()) {
        std::cout << "🏷️  No labels for " << filepath << std::endl;
        return;
    }
    
    std::cout << "\n🏷️  Labels for " << filepath << "\n";
    std::cout << "═══════════════════════════════════════\n";
    
    for (const auto& labelName : labels) {
        auto it = labelDefinitions.find(labelName);
        if (it != labelDefinitions.end()) {
            std::cout << "🏷️  " << labelName << " (" << it->second.color << ")\n";
            std::cout << "   📝 " << it->second.description << "\n";
        } else {
            std::cout << "🏷️  " << labelName << " (no definition)\n";
        }
    }
}

void LabelSystem::showLabelFiles(const std::string& labelName) {
    auto files = getFilesWithLabel(labelName);
    
    if (files.empty()) {
        std::cout << "🏷️  No files with label: " << labelName << std::endl;
        return;
    }
    
    auto it = labelDefinitions.find(labelName);
    if (it != labelDefinitions.end()) {
        std::cout << "\n🏷️  " << labelName << " (" << it->second.color << ")\n";
        std::cout << "📝 " << it->second.description << "\n";
    } else {
        std::cout << "\n🏷️  " << labelName << "\n";
    }
    
    std::cout << "═══════════════════════════════════════\n";
    
    for (const auto& filepath : files) {
        std::cout << "📁 " << filepath << "\n";
    }
}

bool LabelSystem::saveLabels() {
    std::ofstream file(labelsFile);
    if (!file.is_open()) return false;
    
    file << "{\n";
    
    // Save label definitions
    file << "  \"definitions\": {\n";
    bool first = true;
    for (const auto& [name, def] : labelDefinitions) {
        if (!first) file << ",\n";
        file << "    \"" << name << "\": {\n";
        file << "      \"color\": \"" << def.color << "\",\n";
        file << "      \"description\": \"" << def.description << "\"\n";
        file << "    }";
        first = false;
    }
    file << "\n  },\n";
    
    // Save file labels
    file << "  \"file_labels\": {\n";
    first = true;
    for (const auto& [filepath, labels] : fileLabels) {
        if (!first) file << ",\n";
        file << "    \"" << filepath << "\": [";
        
        bool firstLabel = true;
        for (const auto& label : labels) {
            if (!firstLabel) file << ", ";
            file << "\"" << label << "\"";
            firstLabel = false;
        }
        
        file << "]";
        first = false;
    }
    file << "\n  }\n";
    
    file << "}\n";
    return true;
}

bool LabelSystem::loadLabels() {
    // Simplified JSON loading - in a real implementation, use a proper JSON library
    std::ifstream file(labelsFile);
    if (!file.is_open()) return false;
    
    // For now, just create empty structure
    labelDefinitions.clear();
    fileLabels.clear();
    return true;
}

} // namespace gyatt

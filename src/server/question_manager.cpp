#include "question_manager.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <chrono>

QuestionManager::QuestionManager(const std::string& dataFile) 
    : questionDataFile(dataFile), nextQuestionId(1) {
    
    // Initialize random number generator
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    rng.seed(static_cast<unsigned int>(seed));
    
    loadQuestionsFromFile();
    
    // Initialize with default questions if no questions loaded
    if (questions.empty()) {
        initializeDefaultQuestions();
        saveQuestionsToFile();
    }
    
    std::cout << "Question manager initialized with " << questions.size() << " questions." << std::endl;
}

QuestionManager::~QuestionManager() {
    saveQuestionsToFile();
    std::cout << "Question manager shutting down." << std::endl;
}

bool QuestionManager::loadQuestionsFromFile() {
    std::ifstream file(questionDataFile);
    if (!file.is_open()) {
        std::cout << "No existing question file found. Will create default questions." << std::endl;
        return false;
    }
    
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string questionText, option1, option2, option3, option4;
        int questionId, correctAnswerIndex;
        
        if (std::getline(iss, questionText, '|') &&
            std::getline(iss, option1, '|') &&
            std::getline(iss, option2, '|') &&
            std::getline(iss, option3, '|') &&
            std::getline(iss, option4, '|') &&
            iss >> questionId >> correctAnswerIndex) {
            // Check for embedded newlines
            auto has_newline = [](const std::string& s) { return s.find('\n') != std::string::npos || s.find('\r') != std::string::npos; };
            if (has_newline(questionText) || has_newline(option1) || has_newline(option2) || has_newline(option3) || has_newline(option4)) {
                std::cout << "[Warning] Skipping question with embedded newline: " << questionText << std::endl;
                continue;
            }
            std::vector<std::string> options = {option1, option2, option3, option4};
            Question question(questionId, questionText, options, correctAnswerIndex);
            questions.push_back(question);
            questionMap[questionId] = question;
            
            if (questionId >= nextQuestionId) {
                nextQuestionId = questionId + 1;
            }
        }
    }
    
    file.close();
    std::cout << "Loaded " << questions.size() << " questions from file." << std::endl;
    return true;
}

bool QuestionManager::saveQuestionsToFile() const {
    std::ofstream file(questionDataFile);
    if (!file.is_open()) {
        std::cerr << "Failed to open question file for writing." << std::endl;
        return false;
    }
    
    for (const auto& question : questions) {
        file << question.getQuestionText() << "|"
             << question.getOptions()[0] << "|"
             << question.getOptions()[1] << "|"
             << question.getOptions()[2] << "|"
             << question.getOptions()[3] << "|"
             << question.getQuestionId() << " "
             << question.getCorrectAnswerIndex() << std::endl;
    }
    
    file.close();
    return true;
}

bool QuestionManager::addQuestion(const std::string& questionText, 
                                 const std::vector<std::string>& options, 
                                 int correctAnswerIndex) {
    if (options.size() != 4 || correctAnswerIndex < 0 || correctAnswerIndex >= 4) {
        return false;
    }
    
    int questionId = generateQuestionId();
    Question newQuestion(questionId, questionText, options, correctAnswerIndex);
    questions.push_back(newQuestion);
    questionMap[questionId] = newQuestion;
    
    saveQuestionsToFile();
    std::cout << "Question added: " << questionText << " (ID: " << questionId << ")" << std::endl;
    return true;
}

bool QuestionManager::removeQuestion(int questionId) {
    auto it = questionMap.find(questionId);
    if (it == questionMap.end()) {
        return false;
    }
    
    questions.erase(std::remove_if(questions.begin(), questions.end(),
                                  [questionId](const Question& q) { return q.getQuestionId() == questionId; }),
                   questions.end());
    questionMap.erase(it);
    
    saveQuestionsToFile();
    std::cout << "Question removed: " << questionId << std::endl;
    return true;
}

Question* QuestionManager::getQuestion(int questionId) {
    auto it = questionMap.find(questionId);
    if (it != questionMap.end()) {
        return &(it->second);
    }
    return nullptr;
}

Question* QuestionManager::getRandomQuestion() {
    if (questions.empty()) {
        return nullptr;
    }
    
    std::uniform_int_distribution<int> dist(0, static_cast<int>(questions.size()) - 1);
    int randomIndex = dist(rng);
    return &questions[randomIndex];
}

std::vector<Question> QuestionManager::getRandomQuestions(int count) {
    std::vector<Question> result;
    if (questions.empty()) {
        return result;
    }
    
    count = std::min(count, static_cast<int>(questions.size()));
    std::vector<int> indices(questions.size());
    std::iota(indices.begin(), indices.end(), 0);
    
    std::shuffle(indices.begin(), indices.end(), rng);
    
    for (int i = 0; i < count; ++i) {
        result.push_back(questions[indices[i]]);
    }
    
    return result;
}

std::vector<Question> QuestionManager::getAllQuestions() const {
    return questions;
}

bool QuestionManager::validateAnswer(int questionId, int answerIndex) const {
    auto it = questionMap.find(questionId);
    if (it != questionMap.end()) {
        return it->second.isCorrectAnswer(answerIndex);
    }
    return false;
}

bool QuestionManager::questionExists(int questionId) const {
    return questionMap.find(questionId) != questionMap.end();
}

void QuestionManager::initializeDefaultQuestions() {
    std::vector<std::pair<std::string, std::vector<std::string>>> defaultQuestions = {
        {"What is the capital of France?", {"London", "Berlin", "Paris", "Madrid"}},
        {"What is 2 + 2?", {"3", "4", "5", "6"}},
        {"Which planet is closest to the Sun?", {"Venus", "Mercury", "Earth", "Mars"}},
        {"What is the largest ocean on Earth?", {"Atlantic", "Indian", "Arctic", "Pacific"}},
        {"Who wrote Romeo and Juliet?", {"Charles Dickens", "William Shakespeare", "Jane Austen", "Mark Twain"}},
        {"What is the chemical symbol for gold?", {"Ag", "Au", "Fe", "Cu"}},
        {"How many sides does a hexagon have?", {"5", "6", "7", "8"}},
        {"What year did World War II end?", {"1943", "1944", "1945", "1946"}},
        {"What is the main component of the Sun?", {"Liquid lava", "Molten iron", "Hot gases", "Solid rock"}},
        {"Which country is home to the kangaroo?", {"New Zealand", "South Africa", "Australia", "India"}}
    };
    
    std::vector<int> correctAnswers = {2, 1, 1, 3, 1, 1, 1, 2, 2, 2}; // 0-based indices
    
    for (size_t i = 0; i < defaultQuestions.size(); ++i) {
        int questionId = generateQuestionId();
        Question question(questionId, defaultQuestions[i].first, defaultQuestions[i].second, correctAnswers[i]);
        questions.push_back(question);
        questionMap[questionId] = question;
    }
    
    std::cout << "Initialized with " << questions.size() << " default questions." << std::endl;
} 
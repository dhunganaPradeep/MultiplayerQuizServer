#ifndef QUESTION_MANAGER_H
#define QUESTION_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <random>
#include "../common/question.h"

class QuestionManager {
private:
    std::vector<Question> questions;  // All loaded questions
    std::map<int, Question> questionMap;  // questionId -> Question
    std::string questionDataFile;  // File to load questions from
    int nextQuestionId;  // Auto-incrementing question ID
    std::mt19937 rng;  // Random number generator

public:
    QuestionManager(const std::string& dataFile = "data/questions.txt");
    ~QuestionManager();

    // Question loading and management
    bool loadQuestionsFromFile();
    bool saveQuestionsToFile() const;
    bool addQuestion(const std::string& questionText, 
                    const std::vector<std::string>& options, 
                    int correctAnswerIndex);
    bool removeQuestion(int questionId);
    
    // Question serving
    Question* getQuestion(int questionId);
    Question* getRandomQuestion();
    std::vector<Question> getRandomQuestions(int count);
    std::vector<Question> getAllQuestions() const;
    
    // Question validation
    bool validateAnswer(int questionId, int answerIndex) const;
    bool questionExists(int questionId) const;
    
    // Utility
    int getQuestionCount() const { return static_cast<int>(questions.size()); }
    void clearQuestions() { questions.clear(); questionMap.clear(); nextQuestionId = 1; }
    
    // Question ID generation
    int generateQuestionId() { return nextQuestionId++; }
    
    // Initialize with default questions if file is empty
    void initializeDefaultQuestions();
};

#endif // QUESTION_MANAGER_H 
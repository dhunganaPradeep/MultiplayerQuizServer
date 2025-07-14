#ifndef QUESTION_MANAGER_H
#define QUESTION_MANAGER_H

#include <string>
#include <vector>
#include <map>
#include <random>
#include "../common/question.h"

class QuestionManager {
private:
    std::vector<Question> questions;  
    std::map<int, Question> questionMap;
    std::string questionDataFile;  
    int nextQuestionId; 
    std::mt19937 rng;  

public:
    QuestionManager(const std::string& dataFile = "data/questions.txt");
    ~QuestionManager();

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
    
    int getQuestionCount() const { return static_cast<int>(questions.size()); }
    void clearQuestions() { questions.clear(); questionMap.clear(); nextQuestionId = 1; }
    
    int generateQuestionId() { return nextQuestionId++; }
    
    void initializeDefaultQuestions();
};

#endif  
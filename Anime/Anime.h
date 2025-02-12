#ifndef ANIME_H
#define ANIME_H

#include <map>
#include <vector>
#include <string>
#include <memory>
#include <iostream>
#include <algorithm>
#include <functional>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/easing.hpp>
#include <GLFW/glfw3.h>

template <typename T>
class Anime;

template <typename T>
class AnimeClip {
    T& obj;
    std::function<float(float)> easing;
    std::function<void(T&, float)> func;
    AnimeClip<T>* next;
    std::function<void(T&)> onEnd;
    std::function<void(T&)> onStop;
    float startTime;
    float duration;

    bool playing;
    bool repeat;
    bool completed;
public:
    const std::string name;

    AnimeClip(T& obj, const std::string& name, float duration, std::function<float(float)> easing, std::function<void(T&, float)> func)
        : obj(obj), name(name), duration(duration), easing(easing), func(func) {
        startTime = glfwGetTime();
        completed = false;
        playing = true;
        repeat = false;
        next = nullptr;
        onEnd = [](T& o) {};
        onStop = [](T& o) {};
    }

    void step() {
        if (!playing) return;
        float deltaTime = glfwGetTime() - startTime;
        if (deltaTime > duration) deltaTime = duration;

        // (deltaTime / duration) for value beetween 0.0 to 1.0;
        float current = easing(deltaTime / duration);
        func(obj, current);

        if (deltaTime == duration) {
            if (repeat) {
                startTime = glfwGetTime();
            }
            else {
                playing = false;
                completed = true;
                onEnd(obj);
                onStop(obj);
            }
        }
    }

    void stop() {
        playing = false;
        onStop(obj);
    }

    void setup() {
        completed = false;
        playing = true;
        startTime = glfwGetTime();
    }

    void setEnd(std::function<void(T&)> end) {
        onEnd = end;
    }

    void setStop(std::function<void(T&)> stop) {
        onStop = stop;
    }

    void playAfter(AnimeClip<T>* clip) {
        next = clip;
    }

    T& getObj() const {
        return obj;
    }

    AnimeClip<T>* getNext() const {
        return next;
    }

    bool isCompleted() const {
        return completed;
    }

};

class BaseAnime {
public:
    static void processAnimations() {
        for (auto anime : animes) {
            anime->process();
        }
    }
protected:
    static std::vector<BaseAnime*> animes;
    virtual void process() = 0;

    BaseAnime() {
        animes.push_back(this);
    }
};

std::vector<BaseAnime*> BaseAnime::animes;

template <typename T>
class Anime : public BaseAnime {
    static std::map<T*, std::map<std::string, AnimeClip<T>*>> animes;
    static Anime<T>* instance;
public:

    static Anime<T>& getInstance() {
        if (!instance) {
            instance = new Anime<T>();  // Create instance if not exists
        }
        return *instance;
    }

    static AnimeClip<T>* play(T& obj, std::string name, float duration, std::function<float(float)> easing, std::function<void(T&, float)> func) {
        std::map<std::string, AnimeClip<T>*>& clips = getObjectClips(obj);
        if (hasAnimation(obj)) {
            if (clips[name].size() != 0) {
                stopAnime(obj, name);
            }
        }

        getInstance();

        clips[name] = new AnimeClip<T>(obj, name, duration, easing, func);
        clips[name]->setup();
        return clips[name];
    }

    static void play(AnimeClip<T>* clip) {
        std::map<std::string, AnimeClip<T>*>& clips = getObjectClips(clip->getObj());
        if (hasAnimation(clip->getObj())) {
            if (clips[clip->name]) {
                stopAnime(clip->getObj(), clip->name);
            }
        }

        getInstance();

        clips[clip->name] = clip;
        clip->setup();

    }

    static void stopAnime(T& obj, const std::string& name) {
        animes[&obj].erase(name);
    }

    void process() override {
        std::vector<AnimeClip<T>*> completed;
        for (auto& clips : animes) {
            for (auto& clip : clips.second) {
                if (clip.second) {
                    clip.second->step();
                    if (clip.second->isCompleted()) completed.push_back(clip.second);
                }
            }
        }
        for (auto& clip : completed) {
            if (clip->getNext()) play(clip->getNext());
        }
    }

    static bool hasAnimation(T& obj) {
        return animes[&obj].size() > 0;
    }

    static std::map<std::string, AnimeClip<T>*>& getObjectClips(T& obj) {
        return animes[&obj];
    }

    static std::map<std::string, AnimeClip<T>*>& getObjectClips(T* obj) {
        return animes[obj];
    }

private:
    Anime() {}
};

template <typename T>
std::map<T*, std::map<std::string, AnimeClip<T>*>> Anime<T>::animes;

template <typename T>
Anime<T>* Anime<T>::instance;

#endif //ANIME_H

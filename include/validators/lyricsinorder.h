#pragma once

#include <validators/validator.h>

class LyricsInOrder : public Validator
{
public:
    class OrderLyricsAI : public ActionItem {
        std::list<std::unique_ptr<Action>> act;
        int pos_;
    public:
        OrderLyricsAI(int pos);
        QString description() const override;
        Severity severity() const override { return Severity::Warning; }
        const std::list<std::unique_ptr<Action>>& actions() const override;
    };

    virtual std::list<std::shared_ptr<ActionItem>> validate(const Song &song) override;
};


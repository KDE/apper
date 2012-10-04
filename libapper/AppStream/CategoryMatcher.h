#ifndef CATEGORYMATCHER_H
#define CATEGORYMATCHER_H

#include <QMetaType>
#include <QString>
#include <QStringList>
#include <QList>

#include <kdemacros.h>

class KDE_EXPORT CategoryMatcher
{
public:
    typedef enum {
        And,
        Or,
        Not,
        Term
    } Kind;
    explicit CategoryMatcher(Kind kind = And, const QString &term = QString());
    CategoryMatcher(const CategoryMatcher &other);
    ~CategoryMatcher();
    CategoryMatcher& operator=(const CategoryMatcher &other);

    bool match(const QStringList &categories) const;

    void setChild(const QList<CategoryMatcher> &child);
    QList<CategoryMatcher> child() const;
    QString term() const;
    Kind kind() const;

private:
    Kind m_kind;
    QString m_term;
    QList<CategoryMatcher> m_child;
};

Q_DECLARE_METATYPE(CategoryMatcher)

#endif // CATEGORYMATCHER_H

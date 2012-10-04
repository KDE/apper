#include "CategoryMatcher.h"

CategoryMatcher::CategoryMatcher(Kind kind, const QString &term) :
    m_kind(kind),
    m_term(term)
{
}

CategoryMatcher::CategoryMatcher(const CategoryMatcher &other) :
    m_kind(other.m_kind),
    m_term(other.m_term),
    m_child(other.m_child)
{
}

CategoryMatcher::~CategoryMatcher()
{
}

CategoryMatcher &CategoryMatcher::operator =(const CategoryMatcher &other)
{
    m_kind = other.m_kind;
    m_term = other.m_term;
    m_child = other.m_child;
    return *this;
}

bool CategoryMatcher::match(const QStringList &categories) const
{
    if (categories.isEmpty()) {
        return false;
    }

    bool ret = false;
    switch (m_kind) {
    case Term:
        ret = categories.contains(m_term);
        break;
    case And:
        foreach (const CategoryMatcher &parser, m_child) {
            if (!(ret = parser.match(categories))) {
                break;
            }
        }
        break;
    case Or:
        foreach (const CategoryMatcher &parser, m_child) {
            if ((ret = parser.match(categories))) {
                break;
            }
        }
        break;
    case Not:
        // We match like And but negating
        foreach (const CategoryMatcher &parser, m_child) {
            if (!(ret = !parser.match(categories))) {
                break;
            }
        }
        break;
    }
    return ret;
}

void CategoryMatcher::setChild(const QList<CategoryMatcher> &child)
{
    m_child = child;
}

QList<CategoryMatcher> CategoryMatcher::child() const
{
    return m_child;
}

QString CategoryMatcher::term() const
{
    return m_term;
}

CategoryMatcher::Kind CategoryMatcher::kind() const
{
    return m_kind;
}

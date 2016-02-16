#include <QStringList>
#include "tag.h"
#include <QTextDocument>



Tag::Tag(QString text, QString type, int count, QStringList related)
	: m_type(type), m_count(count), m_related(related)
{
	// Decode HTML entities in the tag text
	QTextDocument htmlEncoded;
	htmlEncoded.setHtml(text);
	m_text = htmlEncoded.toPlainText();

	if (type.contains(' '))
	{ m_type = type.left(type.indexOf(' ')); }

	if (text.endsWith("(artist)") && type == "unknown")
	{ m_type = "artist"; }

	if (m_type == "unknown")
	{
		QStringList prep = QStringList() << "artist" << "copyright" << "character" << "model" << "unknown";
		foreach (QString pre, prep)
		{
			if (text.startsWith(pre + ":"))
			{
				m_type = pre;
				m_text = text.mid(pre.length() + 1);
			}
		}
	}
}
Tag::~Tag()
{ }

/**
 * Return the colored tag.
 * @param favs The list of the user's favorite tags.
 * @return The HTML colored tag.
 */
QString Tag::stylished(QList<Favorite> favs) const
{
	for (Favorite fav : favs)
		if (fav.getName() == m_text)
			return "<span style=\"color:pink\">" + m_text + "</span>";

	return m_text;
}

void Tag::setText(QString text)		{ m_text = text;	}
void Tag::setType(QString type)		{ m_type = type;	}
void Tag::setCount(int count)		{ m_count = count;	}
void Tag::setRelated(QStringList r)	{ m_related = r;	}

QString		Tag::text() const		{ return m_text;	}
QString		Tag::type() const		{ return m_type;	}
int			Tag::shortType() const	{ return m_type == "general" ? 0 : (m_type == "artist" ? 1 : (m_type == "copyright" ? 3 : 4)); }
int			Tag::count() const		{ return m_count;	}
QStringList	Tag::related() const	{ return m_related;	}

QString Tag::typedText() const
{
	return (m_type != "general" ? m_type + ":" : "") + m_text;
}

bool sortByFrequency(Tag s1, Tag s2)
{ return s1.count() > s2.count(); }

bool operator==(const Tag &t1, const Tag &t2)
{
	return t1.text() == t2.text() && t1.type() == t2.type();
}

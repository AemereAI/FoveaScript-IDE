#include "syntaxhighlighter.h"

SyntaxHighlighter::SyntaxHighlighter(QTextDocument *parent)
: QSyntaxHighlighter(parent)
{


    HighlightingRule rule;

    keywordFormat.setForeground(Qt::darkBlue);
    keywordFormat.setFontWeight(QFont::Bold);
    const QString keywordPatterns[] = {
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\bas\\b"), QStringLiteral("\\bwhile\\b"),
        QStringLiteral("\\bvar\\b"), QStringLiteral("\\bif\\b"), QStringLiteral("\\belif\\b"),
        QStringLiteral("\\belse\\b"),
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = keywordFormat;
        highlightingRules.append(rule);
    }

    singleLineCommentFormat.setForeground(Qt::yellow);
    rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
    rule.format = singleLineCommentFormat;
    highlightingRules.append(rule);


    functionFormat.setFontItalic(true);
    functionFormat.setForeground(Qt::blue);
    const QString functionPatterns[] = {
        QStringLiteral("\\bcontains\\b"), QStringLiteral("\\bendswith\\b"), QStringLiteral("eval\\b"),
        QStringLiteral("\\bfloat\\b"), QStringLiteral("\\bdict\\b"), QStringLiteral("\\bappend\\b"),
        QStringLiteral("\\bsplit\\b"),QStringLiteral("\\blist\\b"), QStringLiteral("\\bgetElementsBy\\b"),
        QStringLiteral("\\bgetAttribute\\b"), QStringLiteral("\\bgetElementsByAttribute\\b"), QStringLiteral("\\bmultiquote\\b"),
        QStringLiteral("\\bgetImagesBy\\b"), QStringLiteral("\\breadJSON\\b"), QStringLiteral("\\bgetElementsByTag\\b"),
        QStringLiteral("\\bstrip\\b"), QStringLiteral("\\bgetElementsByClass2\\b"), QStringLiteral("\\bgetDictParameter\\b"),
        QStringLiteral("\\bstr\\b"), QStringLiteral("\\blen\\b"), QStringLiteral("\\bleast\\b"),
        QStringLiteral("\\bstripTags\\b"), QStringLiteral("\\bgetPage\\b"), QStringLiteral("\\bpostPage\\b"),
        QStringLiteral("\\bgetElementsByClass\\b"), QStringLiteral("\\bstrReplace\\b"), QStringLiteral("\\bgetHref\\b"),
        QStringLiteral("\\bfindall\\b"), QStringLiteral("\\bgetElementByID\\b"), QStringLiteral("\\bgetElementByID2\\b"),
        QStringLiteral("\\bsendTextPost\\b"), QStringLiteral("\\bsendMultimediaPost\\b"), QStringLiteral("\\bsendCompositePost\\b"),
        QStringLiteral("\\bsendSemanticContent\\b"), QStringLiteral("\\bdisableTypeConversion\\b"), QStringLiteral("\\benableTypeConversion\\b"),
        QStringLiteral("\\bremoveDuplicates\\b"), QStringLiteral("\\bpostForm\\b"), QStringLiteral("\\bcreateJSON\\b"),
        QStringLiteral("\\baddJSONProp\\b"), QStringLiteral("\\bjoinByParameter\\b"), QStringLiteral("\\bidStandardBrowser\\b"),
        QStringLiteral("\\bstripNonTextTags\\b"), QStringLiteral("\\bremoveTag\\b"), QStringLiteral("\\bhasKeywords\\b"),
        QStringLiteral("\\bint\\b"),QStringLiteral("\\benableJavaScript\\b"),QStringLiteral("\\bdisableJavaScript\\b")
    };

    for (const QString &pattern : functionPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = functionFormat;
        highlightingRules.append(rule);
    }

    quotationFormat.setForeground(Qt::darkGreen);
    rule.pattern = QRegularExpression(QStringLiteral("[\"][ A-Za-z0-9_\\.\\/\\-\\\\\\?\\=\\>\\<:]+?[\"]"));
    rule.format = quotationFormat;
    highlightingRules.append(rule);

}

void SyntaxHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : qAsConst(highlightingRules))
    {
        QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), rule.format);
        }
    }
}

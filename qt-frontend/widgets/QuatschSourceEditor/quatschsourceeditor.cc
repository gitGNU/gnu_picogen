//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Copyright (C) 2009  Sebastian Mach (*1983)
// * mail: phresnel/at/gmail/dot/com
// * http://phresnel.org
// * http://picogen.org
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

#include <iostream>
#include "redshift/include/redshift.hh"
#include "quatschsourceeditor.hh"
#include <QCloseEvent>

#include "redshift/include/basictypes/quatsch-height-function.hh"


TextBlockData::TextBlockData()
{
    // Nothing to do
}

QVector<ParenthesisInfo *> TextBlockData::parentheses()
{
    return m_parentheses;
}

void TextBlockData::insert(ParenthesisInfo *info)
{
    int i = 0;
    while (i < m_parentheses.size() &&
        info->position > m_parentheses.at(i)->position)
        ++i;

    m_parentheses.insert(i, info);
}







QuatschSourceEditor::QuatschSourceEditor(QWidget *parent)
: QWidget(parent), ui(new Ui::QuatschEditor), softLock(false)
{
        setAttribute(Qt::WA_DeleteOnClose);
        ui->setupUi(this);

        ui->status->setText("");

        QFont font;
        font.setStyleHint (QFont::TypeWriter, QFont::PreferAntialias);
        font.setFamily("Monospace");
        font.setFixedPitch(true);
        font.setPointSize(9);
        ui->edit->setFont (font);

        highlighter = new QuatschHighlighter (ui->edit->document());
        ui->edit->setText (
                "// Example code:\n"
                "(defun main (u v) (if (< u v) (sin u) (cos v)))\n"
                "\n"
                "(main x y)"
        );
        on_edit_cursorPositionChanged();
        on_edit_textChanged();
        highlighter->rehighlight();
}



void QuatschSourceEditor::setCode(QString code) {
        ui->edit->setText (code);
}



QString QuatschSourceEditor::code () const {
        return ui->edit->toPlainText();
}



void QuatschSourceEditor::on_edit_textChanged () {
        if (softLock)
                return;
        softLock = true;
        QRegExp fundefs ("defun\\s+([A-Za-z\\-_][A-Za-z\\-_0-9]*)");

        const QString code = ui->edit->toPlainText();

        QStringList list;
        int pos = 0;
        while ((pos = fundefs.indexIn(code, pos)) != -1) {
                list << fundefs.cap(1);
                pos += fundefs.matchedLength();
        }

        /*QString str;
        foreach (QString pattern, list) {
                str += pattern;
        }
        ui->status->setText(str);*/
        highlighter->setFunctionNames(list);

        emit codeChanged();
        softLock = false;
}



void QuatschSourceEditor::closeEvent(QCloseEvent *event) {
        Q_UNUSED(event)
}



void QuatschSourceEditor::on_edit_cursorPositionChanged()
{
        const int col = 1 + ui->edit->textCursor().columnNumber();
        const int line = 1 + ui->edit->textCursor().blockNumber();
        ui->position->setText(
                QString::number(line) + ":" + QString::number(col)
        );


    QList<QTextEdit::ExtraSelection> selections;
    ui->edit->setExtraSelections(selections);

    TextBlockData *data = static_cast<TextBlockData *>(
                    ui->edit->textCursor().block().userData());

    if (data) {
        QVector<ParenthesisInfo *> infos = data->parentheses();

        int pos = ui->edit->textCursor().block().position();
        for (int i = 0; i < infos.size(); ++i) {
            ParenthesisInfo *info = infos.at(i);

            int curPos = ui->edit->textCursor().position() -
                         ui->edit->textCursor().block().position();
            if (info->position == curPos - 1 && info->character == '(') {
                if (matchLeftParenthesis(ui->edit->textCursor().block(), i + 1, 0))
                    createParenthesisSelection(pos + info->position);
            } else if (info->position == curPos - 1 && info->character == ')') {
                if (matchRightParenthesis(ui->edit->textCursor().block(), i - 1, 0))
                    createParenthesisSelection(pos + info->position);
            }
        }
    }
}

bool QuatschSourceEditor::matchLeftParenthesis(QTextBlock currentBlock, int i, int numLeftParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> infos = data->parentheses();

    int docPos = currentBlock.position();
    for (; i < infos.size(); ++i) {
        ParenthesisInfo *info = infos.at(i);

        if (info->character == '(') {
            ++numLeftParentheses;
            continue;
        }

        if (info->character == ')' && numLeftParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numLeftParentheses;
    }

    currentBlock = currentBlock.next();
    if (currentBlock.isValid())
        return matchLeftParenthesis(currentBlock, 0, numLeftParentheses);

    return false;
}

bool QuatschSourceEditor::matchRightParenthesis(QTextBlock currentBlock, int i, int numRightParentheses)
{
    TextBlockData *data = static_cast<TextBlockData *>(currentBlock.userData());
    QVector<ParenthesisInfo *> parentheses = data->parentheses();

    int docPos = currentBlock.position();
    for (; i > -1 && parentheses.size() > 0; --i) {
        ParenthesisInfo *info = parentheses.at(i);
        if (info->character == ')') {
            ++numRightParentheses;
            continue;
        }
        if (info->character == '(' && numRightParentheses == 0) {
            createParenthesisSelection(docPos + info->position);
            return true;
        } else
            --numRightParentheses;
    }

    currentBlock = currentBlock.previous();
    if (currentBlock.isValid())
        return matchRightParenthesis(currentBlock, 0, numRightParentheses);

    return false;
}

void QuatschSourceEditor::createParenthesisSelection(int pos)
{
    QList<QTextEdit::ExtraSelection> selections = ui->edit->extraSelections();

    QTextEdit::ExtraSelection selection;
    QTextCharFormat format = selection.format;
    format.setBackground(QBrush(QColor(150,180,150)));
    format.setForeground(QBrush(QColor(200,255,200)));
    format.setFontWeight(QFont::Bold);
    format.setFontUnderline(true);
    selection.format = format;

    QTextCursor cursor = ui->edit->textCursor();
    cursor.setPosition(pos);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
    selection.cursor = cursor;

    selections.append(selection);

    ui->edit->setExtraSelections(selections);
}




#include <QtGui>

QuatschHighlighter::QuatschHighlighter (QTextDocument *parent)
: QSyntaxHighlighter(parent)
{
        HighlightingRule rule;


        functionFormat.setFontItalic(true);
        functionFormat.setForeground(QBrush(QColor(200,255,200)));

        keywordFormat.setForeground(QBrush(QColor(200,200,255)));
        //keywordFormat.setFontWeight(QFont::Bold);
        keywordFormat.setFontItalic(true);
        QStringList keywordPatterns;

        keywordPatterns
                << QRegExp::escape("+")
                << QRegExp::escape("-")
                << QRegExp::escape("*")
                << QRegExp::escape("/")
                << QRegExp::escape("^")
                << QRegExp::escape("<")
                << QRegExp::escape(">")
                << QRegExp::escape("<=")
                << QRegExp::escape(">=")
                << QRegExp::escape("<>")
                << QRegExp::escape("=")
                << QRegExp::escape("!=")
                << QRegExp::escape("[]")
                << QRegExp::escape("[[")
                << QRegExp::escape("]]")
                << QRegExp::escape("][")
                << "\\bmin\\b"
                << "\\bmax\\b"
                << "\\band\\b"
                << "\\bor\\b"
                << "\\blerp\\b"

                << "\\bsin\\b"
                << "\\bcos\\b"
                << "\\bfloor\\b"
                << "\\babs\\b"
                << "\\btrunc\\b"
                << "\\bfrac\\b"
                << "\\bneg\\b"
                << "\\bnot\\b"
                << "\\bsqrt\\b"
                << "\\blog\\b"
                << "\\blog10\\b"
                << "\\bexp\\b"

                << "\\bdelta\\b"
                << "\\bxor\\b"

                << "\\bif\\b"

                << "\\bdefun\\b"
                ;

        foreach (QString pattern, keywordPatterns) {
                rule.pattern = QRegExp(pattern);
                rule.format = keywordFormat;
                highlightingRules.append(rule);
        }

        singleLineCommentFormat.setForeground(QBrush(QColor(255,200,200)));
        rule.pattern = QRegExp("//[^\n]*");
        rule.format = singleLineCommentFormat;
        highlightingRules.append(rule);


        /*
        parametersFormat.setForeground(Qt::darkGreen);
        parametersFormat.setFontWeight(QFont::Bold);
        parametersFormat.setFontItalic(true);
        rule.pattern = QRegExp("\\b[xy]\\b");
        rule.format = parametersFormat;
        highlightingRules.append(rule);*/

        numberFormat.setForeground(Qt::magenta);
        rule.pattern = QRegExp("\\b[0-9]+(\\.[0-9]+)?\\b");
        rule.format = numberFormat;
        highlightingRules.append(rule);

        multiLineCommentFormat.setForeground(QBrush(QColor(255,200,200)));

        commentStartExpression = QRegExp("/\\*");
        commentEndExpression = QRegExp("\\*/");
}



void QuatschHighlighter::setFunctionNames (QStringList names) {
        callHighlightingRules.clear();

        HighlightingRule rule;
        foreach (QString name, names) {
                rule.pattern = QRegExp("\\b" + name + "\\b");
                rule.format = functionFormat;
                callHighlightingRules.append(rule);
        }

        rehighlight(); // should only happen when functions really changed
}



void QuatschHighlighter::highlightBlock(const QString &text) {
        // ++ Qt Quarterly 31
        TextBlockData *data = new TextBlockData;
        int leftPos = text.indexOf('(');
        while (leftPos != -1) {
                ParenthesisInfo *info = new ParenthesisInfo;
                info->character = '(';
                info->position = leftPos;

                data->insert(info);
                leftPos = text.indexOf('(', leftPos + 1);
        }
        int rightPos = text.indexOf(')');
        while (rightPos != -1) {
                ParenthesisInfo *info = new ParenthesisInfo;
                info->character = ')';
                info->position = rightPos;

                data->insert(info);

                rightPos = text.indexOf(')', rightPos +1);
        }
        setCurrentBlockUserData(data);
        // -- Qt Quarterly 31


        foreach (HighlightingRule rule, highlightingRules) {
                QRegExp expression(rule.pattern);
                int index = text.indexOf(expression);
                while (index >= 0) {
                        int length = expression.matchedLength();
                        setFormat(index, length, rule.format);
                        index = text.indexOf(expression, index + length);
                }
        }
        foreach (HighlightingRule rule, callHighlightingRules) {
                QRegExp expression(rule.pattern);
                int index = text.indexOf(expression);
                while (index >= 0) {
                        int length = expression.matchedLength();
                        setFormat(index, length, rule.format);
                        index = text.indexOf(expression, index + length);
                }
        }
        setCurrentBlockState(0);

        int startIndex = 0;
        if (previousBlockState() != 1)
                startIndex = text.indexOf(commentStartExpression);

        while (startIndex >= 0) {
                int endIndex = text.indexOf(commentEndExpression, startIndex);
                int commentLength;
                if (endIndex == -1) {
                        setCurrentBlockState(1);
                        commentLength = text.length() - startIndex;
                } else {
                        commentLength = endIndex - startIndex
                             + commentEndExpression.matchedLength();
                }
                setFormat(startIndex, commentLength, multiLineCommentFormat);
                startIndex = text.indexOf(commentStartExpression,
                                                 startIndex + commentLength);
        }
}


void QuatschSourceEditor::on_compileAndRunButton_clicked() {
        std::stringstream errors;
        try {
                ui->status->setText("");
                redshift::QuatschHeightFunction q (code().toStdString(), errors);
                const unsigned int width = ui->imageSize->value(), height = width;
                QImage image (width, height, QImage::Format_RGB888);


                const float centerX = ui->centerX->value();
                const float centerY = ui->centerY->value();
                const float edgeSize = ui->edgeSize->value();

                float min = std::numeric_limits<float>::infinity(),
                      max = -std::numeric_limits<float>::infinity();
                std::vector<float> values(width*height);
                for (unsigned int y=0; y<height; ++y) {
                        const float v_ = (y/(float)height)-0.5f,
                                    v = (v_*edgeSize) + centerY;

                        for (unsigned int x=0; x<width; ++x) {
                                const float u_ = (x/(float)width)-0.5f,
                                            u = (u_*edgeSize) + centerX;
                                const float h = q(u,v);
                                if (h<min) min=h;
                                if (h>max) max=h;
                                values[x+y*width] = h;
                        }
                }
                const float range = 1 / (max - min);
                for (unsigned int y=0; y<height; ++y) {
                        for (unsigned int x=0; x<width; ++x) {
                                const float h = (values[x+y*width]-min) * range;
                                const int hi_ = h * 255,
                                          hi = hi_<0?0:hi_>255?255:hi_;
                                image.setPixel(x, y, QColor(hi,hi,hi).rgb());
                        }
                }
                //ui->status->setScaledContents(true);
                ui->status->setPixmap(QPixmap::fromImage(image));

        } catch (quatsch::general_exception const &ex) {
                ui->status->setText (QString::fromStdString(
                        ex.getMessage() + ".\n\n"
                        + errors.str()));
        } catch (std::exception const &e) {
                ui->status->setText(e.what());
        } catch (...) {
                ui->status->setText("some weird exception occured");
        }
}

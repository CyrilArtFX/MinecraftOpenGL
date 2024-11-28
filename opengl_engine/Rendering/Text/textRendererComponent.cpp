#include "textRendererComponent.h"
#include <Assets/assetManager.h>
#include <Rendering/Text/font.h>

#include <ServiceLocator/locator.h>
#include <ServiceLocator/renderer.h>


TextRendererComponent::TextRendererComponent() : HudComponent(), text(""), textFont(&AssetManager::GetFont("arial_64")), textSize(Vector2::zero)
{
	Locator::getRenderer().AddText(this);
}

TextRendererComponent::TextRendererComponent(const TextRendererComponent& other) : HudComponent(other), text(other.text), textFont(other.textFont), textSize(other.textSize)
{
	Locator::getRenderer().AddText(this);
}

TextRendererComponent::~TextRendererComponent()
{
	Locator::getRenderer().RemoveText(this);
}


void TextRendererComponent::setTextDatas(const std::string& text_, const Font& textFont_, const Vector2& pivot_, const Vector2& screenPos_, const Vector2& scale_, const float rotAngle_, const Color& tintColor_)
{
	text = text_;
	textFont = &textFont_;

	setHudTransform(pivot_, screenPos_, scale_, rotAngle_);
	setTintColor(tintColor_);

	recomputeTextSize();
}

void TextRendererComponent::setText(const std::string& text_)
{
	text = text_;

	recomputeTextSize();
}

const std::string& TextRendererComponent::getText() const
{
	return text;
}

void TextRendererComponent::setTextFont(const Font& textFont_)
{
	textFont = &textFont_;

	recomputeTextSize();
}

const Font& TextRendererComponent::getTextFont() const
{
	return *textFont;
}

const Vector2& TextRendererComponent::getRawTextSize() const
{
	return textSize;
}


Vector2 TextRendererComponent::getSize() const
{
	return textSize * getScale();
}

bool TextRendererComponent::needToComputeMatrix() const
{
	return false;
}


void TextRendererComponent::recomputeTextSize()
{
	int text_width = 0, text_height = 0;

	std::string text_line;

	//  iterate through all characters of the line of text
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		if (*c == '\n')
		{
			//  if a line return is detected, compute the size of the line and add the line return offset
			FontCharacter ch = textFont->getCharacter(*c);
			text_height += ((ch.Size.y)) * 0.6f;

			computeTextLineSize(text_line, text_width, text_height);
			text_line.clear();
		}
		else
		{
			text_line.push_back(*c);
		}
	}

	//  compute the last line
	computeTextLineSize(text_line, text_width, text_height);

	textSize = Vector2{ (float)(text_width), (float)(text_height) };
}

void TextRendererComponent::computeTextLineSize(std::string textLine, int& textWidth, int& textHeight)
{
	int line_width = 0, max_line_height = 0;

	//  iterate through all characters of the line of text
	std::string::const_iterator c;
	for (c = textLine.begin(); c != textLine.end(); c++)
	{
		FontCharacter ch = textFont->getCharacter(*c);

		line_width += (ch.Advance >> 6); // bitshift by 6 (2^6 = 64) to advance the character size

		max_line_height = Maths::max<int>(max_line_height, ch.Size.y);
	}

	textWidth = Maths::max<int>(textWidth, line_width);
	textHeight += max_line_height;
}

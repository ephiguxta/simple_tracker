import re

def sentence_list(data):
    sentences = []
    regex = re.compile(r"\$([A-Z]{5}),")

    for line in data.split('\n'):
        if line != "":

            match = regex.match(line)
            if match is None:
                break

            sentences.append(match[1])

    # return list(set(sentences))
    return sentences

import re, time, unicodedata, hashlib, urlparse, types, urllib
from pprint import pformat
from urllib2 import HTTPError

# [might want to look into language/country stuff at some point] 
# param info here: http://code.google.com/apis/ajaxsearch/documentation/reference.html
#
CHAPTERDB_URL       = 'https://chapterdb.plex.tv'
CHAPTERDB_BASE      = 'chapters'
CHAPTERDB_SEARCH    = 'search?title='
API_KEY             = 'O98XUZA7ORFGADJR3L1X'

# PlexChapterDB tunables.
SCORE_TITLE_MATCH         = 50  #Score for exact title match
SCORE_DURATION_MATCH      = 10  #Score for matching duration
SCORE_PER_CONFIRM         = 1   #Score per confirmation
SCORE_CONFIRMATION        = 10  #Maximum confirmation score
SCORE_DURATION_SEMI_CLOSE = 10  #Score for duration being within 10%
SCORE_DURATION_CLOSE      = 10  #Score for duration being close
SCORE_CHAPTER_BEYOND_PART = -30 #Score for having chapters beyond the end of the current part
DURATION_MATCH_VAR        = 3   #seconds by which the duration can be off and still considered a match
DURATION_CLOSE_VAR        = 20  #seconds by which the duration can be off and still considered close
CHAPTER_BEYOND_PART_COUNT = 2   #Number of chapters beyond end of part to affect score

# Extras.

class PlexChapterDBAgent():

  def convertTime(self, timeString):
    if (timeString == None):
      return 0
    
    m = re.match('(\d+):(\d+)(?::(\d+))?', timeString)
    if m:
      groups = m.groups()
      if len(groups) == 2:
        time = int(groups[0]) * 60 + int(groups[1])
      else:
        time = int(groups[0]) * 60 * 60 + int(groups[1]) * 60 + int(groups[2])
      return time * 1000
    return None
  
  def cleanChapters(self, searchResult, searchTitle):
    chapterSets = []
    
    for match in searchResult.xpath('//cg:chapterInfo', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'}):
      confirm   = match.get('confirmations')
      language  = match.get('xml:lang')
      title     = match.findtext('cg:title', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'})
      source    = match.find('cg:source', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'})
      chapters  = match.find('cg:chapters', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'})
      
      duration = None
      if source is not None:
        durationText = source.findtext('cg:duration', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'})
        duration = self.convertTime(durationText)
      
      if duration == 0:
        duration = None
        
      score = 0
      if title == searchTitle:
        score += SCORE_TITLE_MATCH
      
      confirmScore = confirm * SCORE_PER_CONFIRM
      if confirmScore > SCORE_CONFIRMATION:
        score += SCORE_CONFIRMATION
      else:
        score += confirmScore
      
      # Defer duration until we start matching parts
      cleanChapters = []
      for chapter in chapters.xpath('cg:chapter', namespaces={'cg': 'http://jvance.com/2008/ChapterGrabber'}):
        timeString  = chapter.get('time')
        name        = chapter.get('name')
        
        time = self.convertTime(timeString)
        
        cleanChapter = {'time': time, 'name': name}
        cleanChapters.append(cleanChapter)
        
      
      chapterSet = {
        'score': score,
        'duration': duration,
        'chapters': cleanChapters
      }
      chapterSets.append(chapterSet)
    return chapterSets
  
  def getPartDuration(self, part):
    duration = 0
    if (hasattr(part, 'duration')):
      duration = int(part.duration)
    
    return duration
  
  def matchPart(self, part, chapterSets):
    duration = self.getPartDuration(part)
    return self.matchDuration(duration, chapterSets)
    
  def matchDuration(self, duration, chapterSets):
    bestScore = -100  #If no match is found, penalize the item that contains this part
    bestChapterSet = None
    
    for chapterSet in chapterSets:
      setScore = chapterSet['score']
      
      setDuration = chapterSet['duration']
      if setDuration != None:
        durationDelta = abs(duration - setDuration)
        if (durationDelta * 100 / setDuration) < 10:
          setScore += SCORE_DURATION_SEMI_CLOSE
        
        if durationDelta < DURATION_CLOSE_VAR * 1000:
          setScore += SCORE_DURATION_CLOSE
      
        if durationDelta < DURATION_MATCH_VAR * 1000:
          setScore += SCORE_DURATION_MATCH
      
      countBeyondDuration = 0
      for chapter in chapterSet['chapters']:
        if chapter['time'] > duration:
          countBeyondDuration += 1
      
      if countBeyondDuration >= CHAPTER_BEYOND_PART_COUNT:
        setScore += SCORE_CHAPTER_BEYOND_PART
      
      if setScore > bestScore:
        bestScore = setScore
        bestChapterSet = chapterSet
    
    return {
      'score': bestScore,
      'chapterSet': bestChapterSet,
      'duration': duration
    };
  
  def update(self, metadata, media, lang):
    searchTitle = media.title
    
    url = '%s/%s/%s%s' % (CHAPTERDB_URL, CHAPTERDB_BASE, CHAPTERDB_SEARCH, urllib.quote(searchTitle))
    searchResult = XML.ElementFromURL(url, cacheTime=CACHE_1WEEK, headers={'Accept-Encoding':'gzip', 'apikey': API_KEY})
    
    chapterSets = self.cleanChapters(searchResult, searchTitle)
    
    bestMatch = None
    for item in media.items:
      match = {
        'score': 0,
        'parts': []
      }
      totalDuration = 0
      for part in item.parts:
        partMatch = self.matchPart(part, chapterSets)
        match['parts'].append(partMatch)
        totalDuration += partMatch['duration']
      
      scoreSum = 0
      count = 0
      for partMatch in match['parts']:
        scoreSum += partMatch['score']
        count += 1
      match['score'] = scoreSum / count
      
      if len(item.parts) != 1:
        #try match whole item as a single part
        itemMatch = self.matchDuration(totalDuration, chapterSets)
        if itemMatch['score'] > match['score']:
          match['score'] = itemMatch['score']
          match['parts'] = [itemMatch]
        
      if bestMatch == None or match['score'] > bestMatch['score']:
        bestMatch = match

    # Clear out old chapters.
    metadata.chapters.clear()
    
    if bestMatch != None and bestMatch['score'] > 0:
      
      offset = 0
      for partMatch in bestMatch['parts']:
        lastChapter = None
        chapterSet = partMatch['chapterSet']
        for matchChapter in chapterSet['chapters']:
          time = matchChapter['time'] + offset
          self.finalizeChapter(lastChapter, time)
          
          chapter = metadata.chapters.new()
          chapter.title = matchChapter['name']
          chapter.start_time_offset = time
          lastChapter = chapter
          
        offset += partMatch['duration']
        self.finalizeChapter(lastChapter, offset)
      
    Log.Debug('Added %d chapters.' % len(metadata.chapters))
  
  def finalizeChapter(self, chapter, endTime):
    if chapter != None:
      chapter.end_time_offset = endTime

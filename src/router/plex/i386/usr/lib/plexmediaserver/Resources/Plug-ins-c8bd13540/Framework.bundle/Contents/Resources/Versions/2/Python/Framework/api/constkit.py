#
#  Plex Extension Framework
#  Copyright (C) 2008-2012 Plex, Inc. (James Clarke, Elan Feingold). All Rights Reserved.
#

import Framework

from base import BaseKit


class ClientPlatforms(Framework.ConstantGroup):

  MacOSX                  = 'MacOSX'
  Linux                   = 'Linux'
  Windows                 = 'Windows'
  iOS                     = 'iOS'
  Android                 = 'Android'
  LGTV                    = 'LGTV'
  Roku                    = 'Roku'
  


class Protocols(Framework.ConstantGroup):

  DASH                    = 'dash'
  HTTP                    = 'http'
  HLS                     = 'hls'
  RTMP                    = 'rtmp'



class OldProtocols(Protocols):

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  Shoutcast               = 'shoutcast'
  WebKit                  = 'webkit'
  HTTPStreamingVideo      = 'http-streaming-video'
  HTTPStreamingVideo720p  = 'http-streaming-video-720p'
  HTTPMP4Video            = 'http-mp4-video'
  HTTPMP4Video720p        = 'http-mp4-video-720p'
  HTTPVideo               = 'http-video'
  RTMP                    = 'rtmp'
  HTTPLiveStreaming       = 'http-live-streaming'
  HTTPMP4Streaming        = 'http-mp4-streaming'
  


class ServerPlatforms(Framework.ConstantGroup):

  MacOSX_i386             = 'MacOSX-i386'
  Linux_i386              = 'Linux-i386'
  Linux_x86_64            = 'Linux-x86_64'
  Linux_MIPS              = 'Linux-MIPS'
  Linux_ARM               = 'Linux-ARM'
  


class ViewTypes(Framework.ConstantGroup):

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  Grid                    = 'grid'
  List                    = 'list'

  

class SummaryTextTypes(Framework.ConstantGroup):

  _excluded_policies = [
    Framework.policies.ModernPolicy,
  ]

  NoSummary               = 0
  Short                   = 1
  Long                    = 2
  


class AudioCodecs(Framework.ConstantGroup):

  AAC                     = 'aac'
  DCA                     = 'dca'
  MP3                     = 'mp3'
  WMA                     = 'wma'
  WMAP                    = 'wmap'
  VORBIS                  = 'vorbis'
  FLAC                    = 'flac'
  


class VideoCodecs(Framework.ConstantGroup):

  H263                    = 'h263'
  H264                    = 'h264'
  VP6                     = 'vp6'
  WVC1                    = 'wvc1'
  DIVX                    = 'divx'
  DIV4                    = 'div4'
  XVID                    = 'xvid'
  THEORA                  = 'theora'


  
class Containers(Framework.ConstantGroup):

  MKV                     = 'mkv'
  MP4                     = 'mp4'
  MPEGTS                  = 'mpegts'
  MOV                     = 'mov'
  AVI                     = 'avi'
  MP3                     = 'mp3'
  OGG                     = 'ogg'
  FLAC                    = 'flac'
  FLV                     = 'flv'


  
class ContainerContents(Framework.ConstantGroup):

  Secondary               = 'secondary'
  Mixed                   = 'mixed'
  Genres                  = 'genre'
  Playlists               = 'playlist'
  Albums                  = 'album'
  Tracks                  = 'track'
  GenericVideos           = 'video'
  Episodes                = 'episode'
  Movies                  = 'movie'
  Seasons                 = 'season'
  Shows                   = 'show'
  Artists                 = 'artist'
  


class StreamTypes(Framework.ConstantGroup):
  
  Video                   = 1
  Audio                   = 2
  Subtitle                = 3
  


class ConstKit(BaseKit):
  
  _root_object = False
  _included_policies = [
    Framework.policies.CodePolicy,
  ]
  _children = [
    ClientPlatforms,
    ServerPlatforms,
    Protocols,
    ViewTypes,
    SummaryTextTypes,
    AudioCodecs,
    VideoCodecs,
    Containers,
    ContainerContents,
  ]
  
  def _init(self):
    self._publish(60, name='CACHE_1MINUTE', excluded_policies=[Framework.policies.CloudPolicy])
    self._publish(3600, name='CACHE_1HOUR', excluded_policies=[Framework.policies.CloudPolicy])
    self._publish(86400, name='CACHE_1DAY', excluded_policies=[Framework.policies.CloudPolicy])
    self._publish(604800, name='CACHE_1WEEK', excluded_policies=[Framework.policies.CloudPolicy])
    self._publish(2592000, name='CACHE_1MONTH', excluded_policies=[Framework.policies.CloudPolicy])
    self._publish(OldProtocols, name='Protocol')


    
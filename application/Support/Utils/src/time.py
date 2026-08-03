import datetime

#-------------------------------------------------------------------------------
def timestamp(nous = True):
    """
    Return current UTC in iso format as as string with or without microseconds.
    """
    return datetime.datetime.utcnow().isoformat()[:(None, 19)[nous]]

#___oOo___

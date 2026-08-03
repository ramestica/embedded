#-------------------------------------------------------------------------------
def binarize(data, threshold = None, fraction = 0.1):
    """
    Logic one means bigger or equal than threshold. If threshold is None
    then compute it as a fraction of the signal range (by default 10%)..
    """
    if threshold == None:
        tmp = min(data)
        threshold = tmp + fraction * (max(data) - tmp)
        
    return [1 if x >= threshold else 0 for x in data]
    
#___oOo___

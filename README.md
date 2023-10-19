# bulk
less than less but more like most

## about
Designed to be a simple and small pager, which should never grow larger than
150k.

### features
* Colored Ouput by default
* RegEx search (not implemented)
* full backwards and forward scrolling

## synposis
```
<command> | bulk [-csw] [--minimal]
bulk [-csw] [--minimal] <file>
bulk --help
bulk --version
```

### options
```
-c --no-color
        Disable colored output. This will convert all coloring Escape codes
        to normal text.

 -s --no-style
        Disable ANSI text formatting like bolds or italics. The affected Escape codes
        will be displayed as normal text.
 
 -w --line-wrapping
        Enables linewrapping
 
 --minimal
        Make bulk appear more like less or more and ditch the most/vim look.
        
 --help
        Show a reduced view of the Synposis and this Option list
 
 --version
        Show the version number of bulk
```

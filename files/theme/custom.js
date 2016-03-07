jQuery(function($) {
  $(document).ready(function() {

    // General styling
    $("body").addClass("postload").addClass("wsite-theme-light").removeClass("wsite-theme-dark");
    
    $('.hamburger').click(function(){
      $("body").toggleClass("menu-open");
    });
    
    // Product Element
    // --------------------------------------------------------------------------------------

    // Swap preview images for hi-res images in product elements

    $("a.wsite-product-image").each(function(){
      var hires = $(this).attr("href");
      $(this).find('img').attr("src", hires);
    });

    // Format Product Element

    function product() {
        $(".wsite-product").each(function() {
          if ($(this).children(".wsite-product-image-wrap").outerHeight() <= $(this).children(".wsite-product-right").outerHeight()) {
            if ($(this).parents('.wsite-multicol-tr').length) {
              $('.wsite-multicol-tr').find(".wsite-product").addClass('break');
            }
            else {
              $(this).addClass('break');
            }
          }
        });
    }

    product();

    var timeout;

    $(window).on('resize', function(e) {

      clearTimeout(timeout);
      timeout = setTimeout(function() {

        product();

      }, 300);

    });

    // End Product Element
    // --------------------------------------------------------------------------------------

    // STORE FRONT PAGE
    // --------------------------------------------------------------------------------------

    // Wrap category text in span for inline wrap effect (will show as block wrap in editor)
    
    $("#wsite-com-store .wsite-com-category-subcategory-name-text").each(function(){
        var category = $(this).text();
        $(this).html('<span>'+category+'</span>');
    });

    $(".splash-page #wsite-content h2, .landing-page #banner p span").each(function(){
        var copy = $(this).html();
        $(this).html('<span>'+copy+'</span>');
    });
    
    // Store category list click
    
    $('.wsite-com-sidebar').click(function(){
        if (!$(this).hasClass('sidebar-expanded')) {
            $(this).addClass('sidebar-expanded');
            if ($('#close').length === 0) {
                $("#wsite-com-hierarchy").prepend('<a id="close" href="#">CLOSE</a>');
                $('#close').click(function(e){
                    e.preventDefault();
                    setTimeout(function() {$('.wsite-com-sidebar').removeClass('sidebar-expanded');}, 50);
                });
            }
        }
    });
    
    
    // Add fullwidth class to gallery thumbs if less than 6

    $('.imageGallery').each(function(){
      if ($(this).children('div').length <= 6) {
        $(this).children('div').addClass('halfwidth-mobile');
      }
    });
    
    // Add swipe to fancybox mobile 

      var swipeGallery = function(){
        setTimeout(function(){
        var touchGallery = document.getElementsByClassName("fancybox-wrap")[0];
        var mc = new Hammer(touchGallery);
        mc.on("panleft panright", function(ev) {
          if (ev.type == "panleft") {
            $("a.fancybox-next").trigger("click");
          }
          else if (ev.type == "panright") {
            $("a.fancybox-prev").trigger("click");
          }
          swipeGallery();
        });
        }, 500);
      }
      if ('ontouchstart' in window) {
        $("body").on( "click", "a.w-fancybox", function() {
          swipeGallery();
        });
    	}
    

    // Format Store markup
    
    $('#wsite-com-product-quantity .wsite-com-product-title').text('Qty');

    $('#wsite-com-product-add-to-cart, .wsite-product-button').removeClass("wsite-button-highlight");

    $("#wsite-com-product-images-strip a:first-child").addClass("current-thumb");
    
    $("#wsite-com-product-images-strip a").click(function(){
        $(".current-thumb").removeClass("current-thumb");
        $(this).addClass("current-thumb");
    });

    // Cart + Member
    
    $('#nav').on('DOMSubtreeModified propertychange', function() {
      
      if ($(window).width() < 992) {
        $("#nav li a").each(function(){
          // Differentiating post-load nav elements by the presence of an id (only currently available modifier)
          if ($(this).attr("id")) {
            var navLinkId = $(this).attr("id");
            var navLinkParent = $(this).parent().detach();

            // Append to mobile nav if new element
            if (!$("#navmobile #"+navLinkId).length) {
              $("#navmobile .wsite-menu-default").append(navLinkParent);
              var newheight = $("#navmobile .wsite-menu-default").height();
              $(".wsite-mobile-menu").height(newheight);
            }
          }
        });
      }

    });
            
    function cartdisplay() {
      if (Number($('#wsite-mini-cart .wsite-subtotal-wrapper .wsite-price').text()) > 0 ) {
        $('#wsite-mini-cart').addClass('full');
      }
      else {
        $('#wsite-mini-cart').removeClass('full');
      }
    }
    
    setTimeout(function() { cartdisplay(); }, 800);
    
    $('.wsite-product-button, #wsite-com-product-add-to-cart, .wsite-product-item .wsite-remove-button').on('click', function(){
      setTimeout(function() { cartdisplay(); }, 800);
    });

    // BLOG
    // --------------------------------------------------------------------------------------
    // --------------------------------------------------------------------------------------

    // Blog styling

    $("#commentReplyTitle").text("Leave a comment");

    $(".blogCommentLevel0").each(function(){
        if ($(this).parent().prev("h2").text() == "Comments") {
            $(this).addClass("first");
        }
    });

    $(".blogCommentLevel1").each(function(){
        if ($(this).parent().prev("div").children("div").hasClass("blogCommentLevel0")) {
            $(this).addClass("first");
        }
    });

    $(".blogCommentLevel2").each(function(){
        if ($(this).parent().prev("div").children("div").hasClass("blogCommentLevel1")) {
            $(this).addClass("first");
        }
    });

  });
});
